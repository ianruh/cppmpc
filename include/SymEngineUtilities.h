// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMENGINEUTILITIES_H_
#define INCLUDE_SYMENGINEUTILITIES_H_

#include <symengine/basic.h>
#include <symengine/matrix.h>
#include <symengine/symbol.h>

#include <map>
#include <string>
#include <unordered_set>

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

typedef std::unordered_set<SymEngine::RCP<const SymEngine::Symbol>,
                           SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq>
        UnorderedSetSymbol;

typedef std::map<RCP<const Basic>, std::string, SymEngine::RCPBasicKeyLess>
        MapBasicString;

// Create a symbol with the `$v_` prefix indicating it is an active variable
RCP<const Symbol> variable(const std::string& name);

// Create a symbol with th `$p_` prefix indicating it is not an active variable
RCP<const Symbol> parameter(const std::string& name);

UnorderedSetSymbol getSymbols(const RCP<const Basic>& basic);

// Get the variables in a given basic. All variables are just symbols whose
// names begin with `$v_`.
//
// For example, a variable named `x`, is a symbol with name `$v_x`.
UnorderedSetSymbol getVariables(const RCP<const Basic>& basic);
UnorderedSetSymbol getVariables(const SymEngine::DenseMatrix& mat);

// Get the parameters in a given basic. All parameters are just symbols whose
// names begin with `$p_`.
//
// For example, a parameter named `i`, is a symbol with name `$p_i`.
UnorderedSetSymbol getParameters(const RCP<const Basic>& basic);
UnorderedSetSymbol getParameters(const SymEngine::DenseMatrix& mat);

// This is a utility just used to test that the swig wrappers can convert to
// and from RCP<const Basic>.
const RCP<const Basic>& echo(const RCP<const Basic>& basic);

/**
 * @brief Expand all elements in the given matrix
 *
 * @param mat The matrix to expand all elememnts in.
 */
void expandAll(SymEngine::DenseMatrix& mat);

/**
 * @brief Generate C code that constructs an eigen matrix equivalent to the
 * passed symbolic matrix, using the given strings as representations for the
 * variables and parameters.
 *
 * @param mat The symbolic mat the matrix eigne matrix is based on.
 * @param variableRepr A map of variables to symbols.
 * @param parameterRepr A map of parameters to symbols.
 * @param matrixName The name of the matrix variable in the generated code.
 */
std::string generateCCode(const SymEngine::DenseMatrix& mat,
                          const MapBasicString& variableRepr,
                          const MapBasicString& parameterRepr,
                          const std::string& matrixName);

}  // namespace cppmpc

#endif  // INCLUDE_SYMENGINEUTILITIES_H_
