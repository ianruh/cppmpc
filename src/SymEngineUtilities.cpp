// Copyright 2021 Ian Ruh
#include "SymEngineUtilities.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

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

}  // namespace cppmpc
