// Copyright 2021 Ian Ruh
#include "SymbolicInequality.h"

#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/constants.h>
#include <symengine/expression.h>
#include <symengine/functions.h>
#include <symengine/matrix.h>
#include <symengine/mul.h>
#include <symengine/polys/basic_conversions.h>
#include <symengine/sets.h>
#include <symengine/subs.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

void SymbolicInequalityConstraints::appendNormalConstraint(
        const RCP<const Basic>& b) {
    this->insertNormalConstraint(this->numConstraints(), b);
}

void SymbolicInequalityConstraints::appendLessThan(const Expression& left,
                                                   const Expression& right) {
    this->insertLessThan(this->numConstraints(), left, right);
}

void SymbolicInequalityConstraints::appendGreaterThan(const Expression& left,
                                                      const Expression& right) {
    this->insertGreaterThan(this->numConstraints(), left, right);
}

void SymbolicInequalityConstraints::removeConstraint(size_t index) {
    this->constraints.erase(this->constraints.begin() + index);
}

const RCP<const Basic>& SymbolicInequalityConstraints::getConstraint(
        size_t index) const {
    return this->constraints.at(index);
}

void SymbolicInequalityConstraints::insertNormalConstraint(
        size_t index, const RCP<const Basic>& b) {
    this->constraints.insert(this->constraints.begin() + index, b);
}

void SymbolicInequalityConstraints::insertLessThan(size_t index,
                                                   const Expression& left,
                                                   const Expression& right) {
    this->insertNormalConstraint(
            index, SymEngine::sub(left.get_basic(), right.get_basic()));
}

void SymbolicInequalityConstraints::insertGreaterThan(size_t index,
                                                      const Expression& left,
                                                      const Expression& right) {
    this->insertNormalConstraint(
            index, SymEngine::sub(right.get_basic(), left.get_basic()));
}

UnorderedSetSymbol SymbolicInequalityConstraints::getSymbols() const {
    UnorderedSetSymbol allSymbols;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(allSymbols, cppmpc::getSymbols(b));
    }
    return allSymbols;
}

UnorderedSetSymbol SymbolicInequalityConstraints::getVariables() const {
    UnorderedSetSymbol variables;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(variables, cppmpc::getVariables(b));
    }
    return variables;
}

UnorderedSetSymbol SymbolicInequalityConstraints::getParameters() const {
    UnorderedSetSymbol parameters;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(parameters, cppmpc::getParameters(b));
    }
    return parameters;
}

RCP<const Basic> SymbolicInequalityConstraints::symbolicBarrierValue() const {
    RCP<const Basic> runningSum = SymEngine::zero;
    for (size_t i = 0; i < this->numConstraints(); i++) {
        RCP<const Basic> withBarrier = SymEngine::neg(
                SymEngine::log(SymEngine::neg(this->getConstraint(i))));
        runningSum = SymEngine::add(runningSum, withBarrier);
    }
    return runningSum;
}

SymEngine::DenseMatrix SymbolicInequalityConstraints::symbolicBarrierGradient(
        const OrderedSet& variableOrdering) const {
    return gradient(this->symbolicBarrierValue(), variableOrdering);
}

SymEngine::DenseMatrix SymbolicInequalityConstraints::symbolicBarrierHessian(
        const OrderedSet& variableOrdering) const {
    return hessian(this->symbolicBarrierValue(), variableOrdering);
}

}  // namespace cppmpc
