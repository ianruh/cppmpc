// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMBOLICEQUALITY_H_
#define INCLUDE_SYMBOLICEQUALITY_H_

#include <symengine/basic.h>
#include <symengine/matrix.h>

#include <utility>
#include <vector>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"

// TODO(ianruh): Identify linear and non-linear terms within a basic.

namespace cppmpc {

using SymEngine::Basic;
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

    void removeConstraint(size_t index);

    const RCP<const Basic>& getConstraint(size_t index) const;

    void insertConstraint(size_t index, const RCP<const Basic>& b);

    size_t numConstraints() const { return this->constraints.size(); }

    UnorderedSetSymbol getSymbols() const;

    UnorderedSetSymbol getVariables() const;

    UnorderedSetSymbol getParameters() const;

    std::pair<SymEngine::DenseMatrix, SymEngine::DenseMatrix>
        convertToLinearSystem(const OrderedSet& variableOrdering) const;
};

}  // namespace cppmpc

#endif  // INCLUDE_SYMBOLICEQUALITY_H_
