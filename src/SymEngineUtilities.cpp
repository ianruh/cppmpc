// Copyright 2021 Ian Ruh
#include "SymEngineUtilities.h"

#include <symengine/basic.h>
#include <symengine/symbol.h>

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

namespace cppmpc {

namespace Utility {

const RCP<const Basic>& echo(const RCP<const Basic>& basic) { return basic; }

}  // namespace Utility

}  // namespace cppmpc
