// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMENGINEUTILITIES_H_
#define INCLUDE_SYMENGINEUTILITIES_H_

#include <symengine/basic.h>
#include <symengine/symbol.h>

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

namespace cppmpc {

namespace Utility {

// This is a utility just used to test that the swig wrappers can convert to
// and from RCP<const Basic>.
const RCP<const Basic>& echo(const RCP<const Basic>& basic);

}  // namespace Utility

}  // namespace cppmpc

bool operator<(const RCP<const Symbol> a, const RCP<const Symbol> b);

#endif  // INCLUDE_SYMENGINEUTILITIES_H_
