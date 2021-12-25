// Copyright 2021 Ian Ruh
#ifndef INCLUDE_GETSYMBOLSVISITOR_H_
#define INCLUDE_GETSYMBOLSVISITOR_H_

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/visitor.h>

#include <unordered_set>

#include "SymEngineUtilities.h"

namespace cppmpc {

// This visitor retrieves all of the symbols in the given basic.
class GetSymbolsVisitor : public SymEngine::BaseVisitor<GetSymbolsVisitor> {
 private:
    UnorderedSetSymbol symbols;

 public:
    GetSymbolsVisitor() {}
    void bvisit(const SymEngine::Symbol &x);
    void bvisit(const SymEngine::Basic &b);

    UnorderedSetSymbol apply(const SymEngine::Basic &b);
};

UnorderedSetSymbol getSymbols(const RCP<const Basic> &basic);

}  // namespace cppmpc

#endif  // INCLUDE_GETSYMBOLSVISITOR_H_
