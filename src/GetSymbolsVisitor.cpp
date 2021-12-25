// Copyright 2021 Ian Ruh
#include "GetSymbolsVisitor.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

#include <unordered_set>
#include <iostream>

using SymEngine::Basic;
using SymEngine::make_rcp;
using SymEngine::RCP;
using SymEngine::Symbol;

namespace cppmpc {

void GetSymbolsVisitor::bvisit(const Symbol &x) {
    this->symbols.insert(SymEngine::symbol(x.get_name()));
}

void GetSymbolsVisitor::bvisit(const Basic &b) {
    std::cout << b << std::endl;
    for(const auto &p : b.get_args()) {
        p->accept(*this);
    }
}

UnorderedSetSymbol GetSymbolsVisitor::apply(const Basic &b) {
    b.accept(*this);
    return this->symbols;
}

UnorderedSetSymbol getSymbols(
        const SymEngine::RCP<const SymEngine::Basic> &basic) {
    GetSymbolsVisitor visitor;
    return visitor.apply(*basic.get());
}

}  // namespace cppmpc
