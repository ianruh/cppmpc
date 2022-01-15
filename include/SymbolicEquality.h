// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMBOLICEQUALITY_H_
#define INCLUDE_SYMBOLICEQUALITY_H_

#include <symengine/basic.h>
#include <symengine/expression.h>
#include <symengine/matrix.h>

#include <string>
#include <utility>
#include <vector>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"

// TODO(ianruh): Identify linear and non-linear terms within a basic.

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::Expression;
using SymEngine::RCP;

class SymbolicEqualityConstraints {
 private:
    std::vector<RCP<const Basic>> constraints;

 public:
    SymbolicEqualityConstraints() {}

    /**
     * @brief Append an equality constraint at the end of the existing
     * constraints.
     *
     * If the top level node of the parameter is not an equality node, then the
     * parameter is assumed to be equal to zero.
     *
     * @param b The constraint to append.
     */
    void appendConstraint(const RCP<const Basic>& b);

    /**
     * @brief Append an equality constraint.
     *
     * @param left The left side of the equality.
     * @param right The right side of the equality.
     */
    void appendConstraint(const Expression& left, const Expression& right);

    void removeConstraint(size_t index);

    const RCP<const Basic>& getConstraint(size_t index) const;

    /**
     * @brief Insert an equality constraint at the given index. If the node
     * is not an equality node, then the node is assumed to be equal to 0.
     *
     * @param index The index to insert at.
     * @param b The node to insert.
     */
    void insertConstraint(size_t index, const RCP<const Basic>& b);

    /**
     * @brief Convenience function to insert an equality constraint based off
     * of two expressions.
     *
     * @param index The index to insert at.
     * @param left Expression for the left side of the equality constraint.
     * @param right Expression for the right side of the equality constraint.
     */
    void insertConstraint(size_t index, const Expression& left,
                          const Expression& right);

    /**
     * @brief The number of equality constraints in.
     */
    size_t numConstraints() const { return this->constraints.size(); }

    UnorderedSetSymbol getSymbols() const;

    UnorderedSetSymbol getVariables() const;

    UnorderedSetSymbol getParameters() const;

    /**
     * @brief Convert the equality constraints to a linear system.
     *
     * @param variableOrdering The variable ordering to use for the linear
     * system.
     *
     * @return A pair containing the  equality matrix, and the constant vector.
     */
    std::pair<SymEngine::DenseMatrix, SymEngine::DenseMatrix>
    convertToLinearSystem(const OrderedSet& variableOrdering) const;

    /**
     * @brief Generate a string with the C++ source code for a function that
     * returns an eigen matrix representing the matrix component of the
     * equality constraints.
     *
     * @param variableOrdering The variable ordering to use for the constraints
     * @param parameterOrdering
     * @param functionName
     */
    std::string generateMatrixFunctionString(
            const OrderedSet& variableOrdering,
            const OrderedSet& parameterOrdering,
            const std::string& functionName) const;
};

}  // namespace cppmpc

#endif  // INCLUDE_SYMBOLICEQUALITY_H_
