#include <iostream>

#include "SymbolicObjective.h"
#include "symengine/basic.h"
#include "symengine/printers.h"
#include "symengine/symbol.h"

int main(int, char**) {
  // cppmpc::SymbolicObjective obj;

  SymEngine::RCP<const SymEngine::Basic> x = SymEngine::symbol("x");

  std::cout << SymEngine::str(*(x.get())) << std::endl;

  return 0;
}
