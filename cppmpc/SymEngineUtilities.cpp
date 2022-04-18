// Copyright 2021 Ian Ruh
#include "SymEngineUtilities.h"

#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/derivative.h>
#include <symengine/dict.h>
#include <symengine/matrix.h>
#include <symengine/mul.h>
#include <symengine/pow.h>
#include <symengine/printers.h>
#include <symengine/subs.h>
#include <symengine/symbol.h>
#include <symengine/expression.h>
#include <symengine/constants.h>
#include <symengine/integer.h>
#include <symengine/functions.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "GetSymbolsVisitor.h"
#include "OrderedSet.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;
using SymEngine::Expression;

RCP<const Symbol> variable(const std::string& name) {
    return SymEngine::symbol("$v_" + name);
}

std::vector<RCP<const Symbol>> variableVector(const std::string& baseName,
                                              size_t num) {
    std::vector<RCP<const Symbol>> vec;
    vec.reserve(num);
    for (size_t i = 0; i < num; i++) {
        vec.push_back(variable(baseName + "[" + std::to_string(i) + "]"));
    }
    return vec;
}

RCP<const Symbol> parameter(const std::string& name) {
    return SymEngine::symbol("$p_" + name);
}

std::vector<RCP<const Symbol>> parameterVector(const std::string& baseName,
                                               size_t num) {
    std::vector<RCP<const Symbol>> vec;
    vec.reserve(num);
    for (size_t i = 0; i < num; i++) {
        vec.push_back(parameter(baseName + "[" + std::to_string(i) + "]"));
    }
    return vec;
}

std::vector<SymEngine::Expression> toExpressions(const std::vector<RCP<const Symbol>>& rcpVec) {
    std::vector<SymEngine::Expression> vec;
    for(const RCP<const Symbol>& symbol: rcpVec) {
        vec.push_back(SymEngine::Expression(symbol));
    }
    return vec;
}

size_t factorial(size_t start) {
    size_t run = 1;
    for(size_t i = start; i != 0; i--) {
        run *= i;
    }
    return run;
}

RCP<const Basic> taylorExpand(const RCP<const Basic>& original,
                              const RCP<const Symbol>& variable,
                              const RCP<const Basic>& location,
                              size_t order) {

    SymEngine::map_basic_basic replaceMap;
    replaceMap[variable] = location;

    RCP<const Basic> terms = SymEngine::zero;
    std::vector<RCP<const Basic>> derivatives = {original};
    for(size_t i = 0; i < order; i++) {
        RCP<const Basic> numerator = SymEngine::xreplace(derivatives.at(derivatives.size()-1), replaceMap);
        RCP<const Basic> denominator = SymEngine::mul(SymEngine::integer(factorial(i)), SymEngine::pow(SymEngine::sub(variable, location), SymEngine::integer(i)));
        terms = SymEngine::add(terms, SymEngine::div(numerator, denominator));

        // Find the ith derivative
        RCP<const Basic> nextDerivative = SymEngine::diff(derivatives.at(derivatives.size()-1), variable);
        derivatives.push_back(nextDerivative);
    }
    
    return terms;
}

RCP<const Basic> taylorExpand(const Expression& original,
                              const Expression& variable,
                              const Expression& location,
                              size_t order) {
    RCP<const Basic> originalBasic = original.get_basic();

    // Check that variable is a symbol
    RCP<const Basic> variableBasic = variable.get_basic();
    if(!SymEngine::is_a<Symbol>(*variableBasic)) {
        throw std::runtime_error("Variable in a taylor expansion must be a symbol.");
    }
    RCP<const Symbol> variableSymbol = SymEngine::rcp_dynamic_cast<const Symbol>(variableBasic);

    RCP<const Basic> locationBasic = location.get_basic();

    return Expression(taylorExpand(originalBasic, variableSymbol, locationBasic, order));
}

SymEngine::DenseMatrix gradient(const RCP<const Basic>& basic,
                                const OrderedSet& variableOrdering) {
    SymEngine::DenseMatrix grad(variableOrdering.size(), 1);
    for (size_t i = 0; i < variableOrdering.size(); i++) {
        RCP<const Symbol> symbol = variableOrdering.at(i);
        grad.set(i, 0, SymEngine::diff(basic, symbol));
    }

    return grad;
}

SymEngine::DenseMatrix jacobian(const SymEngine::DenseMatrix& f,
                                const OrderedSet& variableOrdering) {
    SymEngine::DenseMatrix jacobian(
            variableOrdering.size(),
            variableOrdering.size());
    // Rows first, then columns
    for (size_t row = 0; row < variableOrdering.size(); row++) {
        for(size_t col = 0; col < variableOrdering.size(); col++) {
            RCP<const Symbol> symbol = variableOrdering.at(col);
            jacobian.set(row, col, SymEngine::diff(f.get(row, 0), symbol));
        }
    }

    return jacobian;
}

SymEngine::DenseMatrix hessian(const RCP<const Basic>& basic,
                               const OrderedSet& variableOrdering) {
    SymEngine::DenseMatrix hess(variableOrdering.size(),
                                variableOrdering.size());

    for (size_t row = 0; row < variableOrdering.size(); row++) {
        for (size_t col = 0; col < variableOrdering.size(); col++) {
            RCP<const Symbol> symbol_row = variableOrdering.at(row);
            RCP<const Symbol> symbol_col = variableOrdering.at(col);
            RCP<const Basic> d_row = SymEngine::diff(basic, symbol_row);
            RCP<const Basic> d_row_col = SymEngine::diff(d_row, symbol_col);
            hess.set(row, col, d_row_col);
        }
    }

    return hess;
}

UnorderedSetSymbol getSymbols(const RCP<const Basic>& basic) {
    GetSymbolsVisitor visitor;
    return visitor.apply(*basic.get());
}

UnorderedSetSymbol getVariables(const RCP<const Basic>& basic) {
    UnorderedSetSymbol allSymbols = getSymbols(basic);
    UnorderedSetSymbol variables;
    for (const RCP<const Symbol>& symbol : allSymbols) {
        std::string name = symbol.get()->get_name();
        if (name.substr(0, 3) == "$v_") {
            variables.insert(symbol);
        }
    }
    return variables;
}

UnorderedSetSymbol getVariables(const SymEngine::DenseMatrix& mat) {
    UnorderedSetSymbol allVariables;
    for (size_t row = 0; row < mat.nrows(); row++) {
        for (size_t col = 0; col < mat.ncols(); col++) {
            UnorderedSetSymbol elemenVariables =
                    getVariables(mat.get(row, col));
            allVariables.insert(elemenVariables.begin(), elemenVariables.end());
        }
    }
    return allVariables;
}

UnorderedSetSymbol getParameters(const RCP<const Basic>& basic) {
    UnorderedSetSymbol allSymbols = getSymbols(basic);
    UnorderedSetSymbol parameters;
    for (const RCP<const Symbol>& symbol : allSymbols) {
        std::string name = symbol.get()->get_name();
        if (name.substr(0, 3) == "$p_") {
            parameters.insert(symbol);
        }
    }
    return parameters;
}

UnorderedSetSymbol getParameters(const SymEngine::DenseMatrix& mat) {
    UnorderedSetSymbol allParameters;
    for (size_t row = 0; row < mat.nrows(); row++) {
        for (size_t col = 0; col < mat.ncols(); col++) {
            UnorderedSetSymbol elementParameters =
                    getParameters(mat.get(row, col));
            allParameters.insert(elementParameters.begin(),
                                 elementParameters.end());
        }
    }
    return allParameters;
}

void expandAll(SymEngine::DenseMatrix& mat) {
    for (size_t row = 0; row < mat.nrows(); row++) {
        for (size_t col = 0; col < mat.ncols(); col++) {
            mat.set(row, col, SymEngine::expand(mat.get(row, col)));
        }
    }
}

void util_union(UnorderedSetSymbol& base, const UnorderedSetSymbol& other) {
    for (auto el : other) {
        base.insert(el);
    }
}

const RCP<const Basic>& echo(const RCP<const Basic>& basic) { return basic; }

//============== Convenience Operators/Functions ================

RCP<const Basic> sum(const std::vector<RCP<const Symbol>>& vec) {
    RCP<const Basic> base = SymEngine::zero;
    for (RCP<const Symbol> symbol : vec) {
        base = SymEngine::add(symbol, base);
    }
    return base;
}

RCP<const Basic> norm(const std::vector<RCP<const Symbol>>& vec) {
    RCP<const Basic> base = SymEngine::zero;
    for (RCP<const Symbol> symbol : vec) {
        base = SymEngine::add(SymEngine::mul(symbol, symbol), base);
    }
    return SymEngine::sqrt(base);
}

RCP<const Basic> squaredSum(const std::vector<RCP<const Symbol>>& vec) {
    RCP<const Basic> base = SymEngine::zero;
    for (RCP<const Symbol> symbol : vec) {
        base = SymEngine::add(SymEngine::mul(symbol, symbol), base);
    }
    return base;
}

Expression sin(const Expression& arg) {
    return Expression(SymEngine::sin(arg.get_basic()));
}

Expression cos(const Expression& arg) {
    return Expression(SymEngine::cos(arg.get_basic()));
}

Expression tan(const Expression& arg) {
    return Expression(SymEngine::tan(arg.get_basic()));
}

Expression pow(const Expression& base, const Expression& order) {
    return Expression(SymEngine::pow(base.get_basic(), order.get_basic()));
}

}  // namespace cppmpc
