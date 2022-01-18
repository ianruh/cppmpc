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

RCP<const Symbol> variable(const std::string& name) {
    return SymEngine::symbol("$v_" + name);
}

std::vector<RCP<const Symbol>> variableVector(const std::string& baseName,
                                              size_t num) {
    std::vector<RCP<const Symbol>> vec(num);
    for (size_t i = 0; i < num; i++) {
        vec.push_back(variable(baseName + std::to_string(i)));
    }
    return vec;
}

RCP<const Symbol> parameter(const std::string& name) {
    return SymEngine::symbol("$p_" + name);
}

std::vector<RCP<const Symbol>> parameterVector(const std::string& baseName,
                                               size_t num) {
    std::vector<RCP<const Symbol>> vec(num);
    for (size_t i = 0; i < num; i++) {
        vec.push_back(parameter(baseName + std::to_string(i)));
    }
    return vec;
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

}  // namespace cppmpc
