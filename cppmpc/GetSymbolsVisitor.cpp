// Copyright 2021 Ian Ruh
#include "GetSymbolsVisitor.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

#include <iostream>
#include <unordered_set>

#include "SymEngineUtilities.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

void GetSymbolsVisitor::bvisit(const SymEngine::Symbol &x) {
    this->symbols.insert(SymEngine::symbol(x.get_name()));
}

void GetSymbolsVisitor::bvisit(const SymEngine::Basic &b) {
    for (const auto &p : b.get_args()) {
        p->accept(*this);
    }
}

UnorderedSetSymbol GetSymbolsVisitor::apply(const SymEngine::Basic &b) {
    b.accept(*this);
    return this->symbols;
}

}  // namespace cppmpc
