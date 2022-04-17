// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMBOLICINEQUALITY_H_
#define INCLUDE_SYMBOLICINEQUALITY_H_

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

class SymbolicInequalityConstraints {
 private:
    std::vector<RCP<const Basic>> constraints;

 public:
    SymbolicInequalityConstraints() {}

    /**
     * @brief Append an inequality constraint at the end of the existing
     * constraints.
     *
     * The constraint is expected to be in normal form, e.g. basic < 0
     *
     * @param b The constraint to append.
     */
    void appendNormalConstraint(const RCP<const Basic>& b);

    /**
     * @brief Append a less than constraint.
     *
     * @param left The left side of the less than.
     * @param right The right side of the less than.
     */
    void appendLessThan(const Expression& left, const Expression& right);

    /**
     * @brief Append a greater than constraint.
     *
     * @param left The left side of the greater than.
     * @param right The right side of the greater than.
     */
    void appendGreaterThan(const Expression& left, const Expression& right);

    /**
     * @brief Remove the given constraint.
     *
     * @param index The index to remove.
     */
    void removeConstraint(size_t index);

    /**
     * @brief Get the constraint with the given index.
     */
    const RCP<const Basic>& getConstraint(size_t index) const;

    /**
     * @brief Insert an inequality constraint at the given index.
     *
     * The node should be in normal form.
     *
     * @param index The index to insert at.
     * @param b The node to insert.
     */
    void insertNormalConstraint(size_t index, const RCP<const Basic>& b);

    /**
     * @brief Convenience function to insert an inequality constraint based off
     * of two expressions.
     *
     * @param index The index to insert at.
     * @param left Expression for the left side of the less than.
     * @param right Expression for the right side of the less than.
     */
    void insertLessThan(size_t index, const Expression& left,
                        const Expression& right);

    /**
     * @brief Convenience function to insert an inequality constraint based off
     * of two expressions.
     *
     * @param index The index to insert at.
     * @param left Expression for the left side of the greater than.
     * @param right Expression for the right side of the greater than.
     */
    void insertGreaterThan(size_t index, const Expression& left,
                           const Expression& right);

    /**
     * @brief The number of inequality constraints.
     */
    size_t numConstraints() const { return this->constraints.size(); }

    UnorderedSetSymbol getSymbols() const;

    UnorderedSetSymbol getVariables() const;

    UnorderedSetSymbol getParameters() const;

    RCP<const Basic> symbolicBarrierValue() const;

    SymEngine::DenseMatrix symbolicBarrierGradient(
            const OrderedSet& variableOrdering) const;

    SymEngine::DenseMatrix symbolicBarrierHessian(
            const OrderedSet& variableOrdering) const;
};

}  // namespace cppmpc

#endif  // INCLUDE_SYMBOLICINEQUALITY_H_
