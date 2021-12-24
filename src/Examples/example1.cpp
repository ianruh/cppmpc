#include "SymbolicObjective.hpp"
#include "symengine/basic.h"
#include "symengine/symbol.h"
#include "symengine/printers.h"

#include <iostream>

int main(int, char**) {
    //cppmpc::SymbolicObjective obj;

    SymEngine::RCP<const SymEngine::Basic> x = SymEngine::symbol("x");

    std::cout << SymEngine::str(*(x.get())) << std::endl;

    return 0;
}
