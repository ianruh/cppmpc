#ifndef _GET_SYMBOLS_VISITOR_H
#define _GET_SYMBOLS_VISITOR_H

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/visitor.h>

#include <unordered_set>

namespace cppmpc {

typedef std::unordered_set<SymEngine::RCP<const SymEngine::Basic>,
                           SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq>
    UnorderedSetBasic;

typedef std::unordered_set<SymEngine::RCP<const SymEngine::Symbol>,
                           SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq>
    UnorderedSetSymbol;

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

// This visitor retrieves all of the symbols in the given basic.
class GetSymbolsVisitor : public SymEngine::BaseVisitor<GetSymbolsVisitor> {
 private:
  UnorderedSetSymbol symbols;

 public:
  GetSymbolsVisitor(){};
  void bvisit(const Symbol &x);
  void bvisit(const Basic &){};

  UnorderedSetSymbol apply(const Basic &b);
};

}  // namespace cppmpc

#endif  // _GET_SYMBOLS_VISITOR_H
