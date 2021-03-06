// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMENGINEUTILITIES_H_
#define INCLUDE_SYMENGINEUTILITIES_H_

#include <symengine/basic.h>
#include <symengine/matrix.h>
#include <symengine/symbol.h>
#include <symengine/expression.h>

#include <map>
#include <string>
#include <unordered_set>

#include "OrderedSet.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;
using SymEngine::Expression;

typedef std::unordered_set<SymEngine::RCP<const SymEngine::Symbol>,
                           SymEngine::RCPBasicHash, SymEngine::RCPBasicKeyEq>
        UnorderedSetSymbol;

typedef std::map<RCP<const Basic>, std::string, SymEngine::RCPBasicKeyLess>
        MapBasicString;

/**
 * @brief Create a symbol with the `$v_` prefix indicating it is an active
 * variable.
 *
 * @param name The name of the variable.
 */
RCP<const Symbol> variable(const std::string& name);
std::vector<RCP<const Symbol>> variableVector(const std::string& baseName,
                                              size_t num);

/**
 * @brief Create a symbol with the `$p_` prefix indicating it is not an active
 * variable.
 *
 * @param name The name of the parameter.
 */
RCP<const Symbol> parameter(const std::string& name);
std::vector<RCP<const Symbol>> parameterVector(const std::string& baseName,
                                               size_t num);

std::vector<SymEngine::Expression> toExpressions(const std::vector<RCP<const Symbol>>& rcpVec);

/**
 * @brief The SymEngine::series function implicitly only expands a series
 * about 0, which isn't good enough for us. This does some substitutions
 * to taylor expand the given basic about the given symbol at any location.
 *
 * @param original [TODO:description]
 * @param variable [TODO:description]
 * @param location [TODO:description]
 * @param order [TODO:description]
 */
RCP<const Basic> taylorExpand(const RCP<const Basic>& original,
                                         const RCP<const Symbol>& variable,
                                         const RCP<const Basic>& location,
                                         size_t order);

RCP<const Basic> taylorExpand(const Expression& original,
                              const Expression& variable,
                              const Expression& location,
                              size_t order);

/**
 * @brief Get the gradient of the basic using the given variable ordering.
 *
 * @param basic The basic.
 * @param variableOrdering The variable ordering.
 *
 * @return A 1xn matrix contatining the gradient of the basic.
 */
SymEngine::DenseMatrix gradient(const RCP<const Basic>& basic,
                                const OrderedSet& variableOrdering);

/**
 * @brief Get the jacobian of the vector valued functionn f.
 *
 * @param f A vector valued function.
 * @param variableOrdering The variable ordering to use.
 *
 * @return The jacobian of the function f.
 */
SymEngine::DenseMatrix jacobian(const SymEngine::DenseMatrix& f,
                                const OrderedSet& variableOrdering);

/**
 * @brief Get the hessian of the basic using the given variable ordering.
 *
 * @param basic The basic.
 * @param variableOrdering The variable ordering.
 *
 * @return An nxn matrix containing the hessian of the basic.
 */
SymEngine::DenseMatrix hessian(const RCP<const Basic>& basic,
                               const OrderedSet& variableOrdering);

/**
 * @brief Get all the symbols present in a basic.
 *
 * @param basic The basic
 * @return The unordered set of symbols in the basic.
 */
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

/**
 * @brief Take an inplace union of twp unordered sets.
 *
 * @param base The set to get stuff inserted into.
 * @param other The set to insert into the base.
 */
void util_union(UnorderedSetSymbol& base, const UnorderedSetSymbol& other);

// This is a utility just used to test that the swig wrappers can convert to
// and from RCP<const Basic>.
const RCP<const Basic>& echo(const RCP<const Basic>& basic);

/**
 * @brief Expand all elements in the given matrix
 *
 * @param mat The matrix to expand all elememnts in.
 */
void expandAll(SymEngine::DenseMatrix& mat);

//============== Convenience Operators/Functions ================

// Need expression versions of all of these
RCP<const Basic> sum(const std::vector<RCP<const Symbol>>& vec);
RCP<const Basic> norm(const std::vector<RCP<const Symbol>>& vec);
RCP<const Basic> squaredSum(const std::vector<RCP<const Symbol>>& vec);

Expression sin(const Expression& arg);
Expression cos(const Expression& arg);
Expression tan(const Expression& arg);
Expression pow(const Expression& base, const Expression& order);

}  // namespace cppmpc

#endif  // INCLUDE_SYMENGINEUTILITIES_H_
