// Copyright 2021 Ian Ruh
#include "GetSymbolsVisitor.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

#include <unordered_set>

using SymEngine::Basic;
using SymEngine::make_rcp;
using SymEngine::RCP;
using SymEngine::Symbol;

namespace cppmpc {

void GetSymbolsVisitor::bvisit(const Symbol &x) {
    this->symbols.insert(make_rcp<const Symbol>(x.get_name()));
}

UnorderedSetSymbol GetSymbolsVisitor::apply(const Basic &b) {
    b.accept(*this);
    return this->symbols;
}
}  // namespace cppmpc
