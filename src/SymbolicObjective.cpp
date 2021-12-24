#include <iostream>
#include "SymbolicObjective.hpp"

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/number.h>
#include <symengine/add.h>
#include <symengine/mul.h>

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::mul;
using SymEngine::symbol;

namespace cppmpc {
    
    SymbolicObjective::SymbolicObjective () {
        std::cout << "Hello World" << std::endl;
    }

    void SymbolicObjective::print(const RCP<const Basic> &basic) {
        std::cout << *basic.ptr() << std::endl;
    }

} // namespace cppmpc
