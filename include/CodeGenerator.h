// Copyright 2021 Ian Ruh
#ifndef INCLUDE_CODEGENERATOR_H_
#define INCLUDE_CODEGENERATOR_H_

#include <symengine/basic.h>
#include "symengine/matrix.h"

#include <exception>
#include <string>
#include <tuple>
#include <vector>
#include "OrderedSet.h"
#include "SymbolicEquality.h"
#include "SymbolicInequality.h"
#include "Util.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;

class CodeGenerator {
 public:
    /**
     * @brief Generate C code that constructs an eigen matrix equivalent to the
     * passed symbolic matrix, using the given strings as representations for
     * the variables and parameters.
     *
     * The matrix elements are set in columns major order.
     *
     * @param mat The symbolic mat the matrix eigne matrix is based on.
     * @param variableRepr A map of variables to symbols.
     * @param parameterRepr A map of parameters to symbols.
     * @param matrixName The name of the matrix variable in the generated code.
     */
    static std::string generateDenseMatrixCode(
            const SymEngine::DenseMatrix& mat,
            const MapBasicString& variableRepr,
            const MapBasicString& parameterRepr, const std::string& matrixName);

    /**
     * @brief Generate code that can be compiled and used to calculate the
     * objective value, gradient, and hessian.
     *
     * @param symbolicObjective The symbolic objective basic.
     * @param variableOrdering The variable ordering.
     * @param parameterOrdering The parameter ordering.
     * @param valueFunctionName Value function name.
     * @param gradientFunctionName Gradient function name.
     * @param hessianFunctionName Hessian function name.
     */
    static std::tuple<std::string, std::string, std::string>
    generateObjectiveFunctions(const RCP<const Basic> symbolicObjective,
                               const SymEngine::DenseMatrix& gradientMat,
                               const SymEngine::DenseMatrix& hessianMat,
                               const OrderedSet& variableOrdering,
                               const OrderedSet& parameterOrdering,
                               const std::string& valueFunctionName,
                               const std::string& gradientFunctionName,
                               const std::string& hessianFunctionName);

    /**
     * @brief Generate code that can be compiled and used to calculate the
     * equality matrix and vector.
     *
     * @param symbolicConstraints The symbolic equality constraints to generate
     * code for
     * @param variableOrdering Variable ordering to use.
     * @param parameterOrdering Parameter ordering to use.
     * @param functionName The name of the function to generate.
     */
    static std::pair<std::string, std::string>
    generateSymbolicEqualityFunctions(
            const SymbolicEqualityConstraints& symbolicConstraints,
            const OrderedSet& variableOrdering,
            const OrderedSet& parameterOrdering,
            const std::string& matrixFunctionName,
            const std::string& vectorFunctionName);

    /**
     * @brief Generate code that can be compiled and used to calculate the
     * inequality constraint functions.
     *
     * @param symbolicConstraints The symbolic inequality constraints.
     * @param variableOrdering Variable orderinng
     * @param parameterOrdering Parameter ordering
     * @param valueFunctionName The function name for the value function
     * @param gradientFunctionName The function name for the gradient function
     * @param hessianFunctionName The functionn name for the hessian function
     */
    static std::tuple<std::string, std::string, std::string>
    generateSymbolicInequalityFunctions(
            const SymbolicInequalityConstraints& symbolicConstraints,
            const OrderedSet& variableOrdering,
            const OrderedSet& parameterOrdering,
            const std::string& valueFunctionName,
            const std::string& gradientFunctionName,
            const std::string& hessianFunctionName);

    static void writeFunctionsToFile(
            const std::string& filePath,
            const std::vector<std::string>& functionStrings);
};

}  // namespace cppmpc

#endif  // INCLUDE_CODEGENERATOR_H_
