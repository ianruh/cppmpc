// Copyright 2021 Ian Ruh
#include "SymbolicObjective.h"

#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/mul.h>
#include <symengine/number.h>
#include <symengine/printers.h>
#include <symengine/symbol.h>

#include <iostream>

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::mul;
using SymEngine::symbol;

namespace cppmpc {

SymbolicObjective::SymbolicObjective() {
    std::cout << "Hello World" << std::endl;
}

}  // namespace cppmpc
