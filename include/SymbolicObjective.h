// Copyright 2021 Ian Ruh
#ifndef INCLUDE_SYMBOLICOBJECTIVE_H_
#define INCLUDE_SYMBOLICOBJECTIVE_H_

#include <optional>
#include <string>

#include <symengine/basic.h>
#include <symengine/expression.h>

#include "CodeGenerator.h"
#include "FastMPCFunctionPointerObjective.h"
#include "OrderedSet.h"
#include "SymEngineUtilities.h"
#include "SymbolicEquality.h"
#include "SymbolicInequality.h"

using SymEngine::Basic;
using SymEngine::RCP;

namespace cppmpc {

namespace FastMPC {

class SymbolicObjective : public FunctionPointerObjective {
 private:
    std::optional<RCP<const Basic>> objective;

    bool finalized = false;

    // Function names
    const std::string valueFunctionName = "value";
    const std::string gradientFunctionName = "gradient";
    const std::string hessianFunctionName = "hessian";
    const std::string equalityMatrixFunctionName = "equalityMatrix";
    const std::string equalityVectorFunctionName = "equalityVector";
    const std::string inequalityValueFunctionName = "inequalityValue";
    const std::string inequalityGradientFunctionName = "inequalityGradient";
    const std::string inequalityHessianFunctionName = "inequalityHessian";

 protected:
    /**
     * @brief Check that the symbolic objective has been finalized and call
     * the function pointer objective validate function.
     */
    std::optional<std::string> validate() const override;

 public:
    SymbolicObjective();

    SymbolicEqualityConstraints equalityConstraints;
    SymbolicInequalityConstraints inequalityConstraints;

    /**
     * @brief Set the objective function
     */
    void setObjective(RCP<const Basic> obj);
    void setObjective(SymEngine::Expression obj);

    /**
     * @brief Compile the objectives and set the function pointers.
     *
     * @param variableOrdering [TODO:description]
     * @param parameterOrdering [TODO:description]
     */
    void finalize(const OrderedSet& variableOrdering,
                  const OrderedSet& parameterOrdering);

    UnorderedSetSymbol getSymbols() const;

    UnorderedSetSymbol getVariables() const;

    UnorderedSetSymbol getParameters() const;

 public:
    int numParameters() const override;
    int numVariables() const override;
    int numInequalityConstraints() const override;
    int numEqualityConstraints() const override;
};

}  // namespace FastMPC

}  // namespace cppmpc

#endif  // INCLUDE_SYMBOLICOBJECTIVE_H_
