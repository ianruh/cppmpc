// Copyright 2021 Ian Ruh
#include "SymEngineUtilities.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

#include <string>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <stdexcept>

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

const RCP<const Basic>& echo(const RCP<const Basic>& basic) { return basic; }


std::string generateCCode(const SymEngine::DenseMatrix& mat,
            const MapBasicString& variableRepr,
            const MapBasicString& parameterRepr) {
    // Verify that each variable  in the matrix has a representation
    // Verify that each parameter in the matrix has a representation
    for(size_t i = 0; i < mat.nrows(); i++) {
        for(size_t j = 0; j < mat.ncols(); j++) {
            UnorderedSetSymbol variables = getVariables(mat.get(i, j));
            UnorderedSetSymbol parameters = getParameters(mat.get(i, j));

            for(auto variable: variables) {
                if(variableRepr.count(variable) != 1) {
                    throw std::runtime_error("A representation for a variable was not found");
                }
            }
            for(auto parameter: parameters) {
                if(parameterRepr.count(parameter) != 1) {
                    throw std::runtime_error("A representation for a parameter was not found");
                }
            }
        }
    }

    std::stringstream ss;

    ss << "Eigen::MatrixXd m(" << mat.nrows() << "," << mat.ncols() << ");" << std::endl;

    return ss.str();
}

}  // namespace cppmpc
