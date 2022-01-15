// Copyright 2021 Ian Ruh
#include "SymEngineUtilities.h"

#include <symengine/basic.h>
#include <symengine/dict.h>
#include <symengine/matrix.h>
#include <symengine/printers.h>
#include <symengine/subs.h>
#include <symengine/symbol.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "GetSymbolsVisitor.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

RCP<const Symbol> variable(const std::string& name) {
    return SymEngine::symbol("$v_" + name);
}

RCP<const Symbol> parameter(const std::string& name) {
    return SymEngine::symbol("$p_" + name);
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

const RCP<const Basic>& echo(const RCP<const Basic>& basic) { return basic; }

std::string generateCCode(const SymEngine::DenseMatrix& mat,
                          const MapBasicString& variableRepr,
                          const MapBasicString& parameterRepr,
                          const std::string& matrixName) {
    // Verify that each variable  in the matrix has a representation
    // Verify that each parameter in the matrix has a representation
    for (size_t i = 0; i < mat.nrows(); i++) {
        for (size_t j = 0; j < mat.ncols(); j++) {
            UnorderedSetSymbol variables = getVariables(mat.get(i, j));
            UnorderedSetSymbol parameters = getParameters(mat.get(i, j));

            for (auto variable : variables) {
                if (variableRepr.count(variable) != 1) {
                    throw std::runtime_error(
                            "A representation for a variable was not found");
                }
            }
            for (auto parameter : parameters) {
                if (parameterRepr.count(parameter) != 1) {
                    throw std::runtime_error(
                            "A representation for a parameter was not found");
                }
            }
        }
    }

    // Now that we have checked, we can throw them all together
    MapBasicString symbolsMap;
    symbolsMap.insert(variableRepr.begin(), variableRepr.end());
    symbolsMap.insert(parameterRepr.begin(), parameterRepr.end());

    // Construct the map of symbols to replace
    SymEngine::map_basic_basic symbolsRepMap;
    MapBasicString::iterator it;
    for (it = symbolsMap.begin(); it != symbolsMap.end(); it++) {
        symbolsRepMap[it->first] = SymEngine::symbol(it->second);
    }

    std::stringstream ss;

    ss << "Eigen::MatrixXd " << matrixName << "(" << mat.nrows() << ","
       << mat.ncols() << ");" << std::endl;
    for (size_t row = 0; row < mat.nrows(); row++) {
        for (size_t col = 0l; col < mat.ncols(); col++) {
            RCP<const Basic> replaced = SymEngine::expand(
                    SymEngine::xreplace(mat.get(row, col), symbolsRepMap));
            ss << matrixName << "(" << row << "," << col
               << ") = " << SymEngine::ccode(*replaced) << ";" << std::endl;
        }
    }

    return ss.str();
}

}  // namespace cppmpc
