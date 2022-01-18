// Copyright 2021 Ian Ruh
#include "SymbolicObjective.h"

#include <dlfcn.h>
#include <symengine/basic.h>
#include <symengine/matrix.h>
#include <symengine/symbol.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "FastMPCFunctionPointerObjective.h"
#include "SymEngineUtilities.h"
#include "Util.h"

namespace cppmpc {

namespace FastMPC {

using SymEngine::Basic;
using SymEngine::RCP;

/**
 * @brief We set all of the parameters equal to negative 1. These act as flags
 * to indicate that we should use the symbolic constraints/objectives to get
 * the values.
 *
 */
SymbolicObjective::SymbolicObjective()
        : FunctionPointerObjective(-1, -1, -1, -1) {}

std::optional<std::string> SymbolicObjective::validate() const {
    // Check that we have been finalized
    if (!this->finalized) {
        return "SymbolicObjective must be finalized before being givenn to a "
               "solver.";
    }
    return FunctionPointerObjective::validate();
}

void SymbolicObjective::setObjective(RCP<const Basic> obj) {
    this->objective = obj;
}

void SymbolicObjective::setObjective(SymEngine::Expression obj) {
    this->setObjective(obj.get_basic());
}

void SymbolicObjective::finalize(const OrderedSet& variableOrdering,
                                 const OrderedSet& parameterOrdering) {
    // Check that we have the objective
    if (!this->objective) {
        throw std::runtime_error(
                "Objective must be set before it can be finalized.");
    }

    // Get the gradient and the hessian of the objective
    SymEngine::DenseMatrix symbolicObjectiveGradient =
            cppmpc::gradient(*this->objective, variableOrdering);
    SymEngine::DenseMatrix symbolicObjectiveHessian =
            cppmpc::hessian(*this->objective, variableOrdering);

    // Vector to store all of the function strings
    std::vector<std::string> functionStrings(8, "");

    //====== Objective Functions ======
    std::tie(functionStrings[0], functionStrings[1], functionStrings[2]) =
            CodeGenerator::generateObjectiveFunctions(
                    *this->objective, symbolicObjectiveGradient,
                    symbolicObjectiveHessian, variableOrdering,
                    parameterOrdering, this->valueFunctionName,
                    this->gradientFunctionName, this->hessianFunctionName);

    //====== Equality Functions ======
    std::tie(functionStrings[3], functionStrings[4]) =
            CodeGenerator::generateSymbolicEqualityFunctions(
                    this->equalityConstraints, variableOrdering,
                    parameterOrdering, this->equalityMatrixFunctionName,
                    this->equalityVectorFunctionName);

    //====== Inequality Functions ======
    std::tie(functionStrings[5], functionStrings[6], functionStrings[7]) =
            CodeGenerator::generateSymbolicInequalityFunctions(
                    this->inequalityConstraints, variableOrdering,
                    parameterOrdering, this->inequalityValueFunctionName,
                    this->inequalityGradientFunctionName,
                    this->inequalityHessianFunctionName);

    std::string tempFileBase = std::tmpnam(nullptr);
    std::string tempFile = tempFileBase + std::string(".cpp");

    CodeGenerator::writeFunctionsToFile(tempFile, functionStrings);

    std::string tempSharedObject = tempFileBase + std::string(".so");

    std::stringstream cmd;
    cmd << CPP_COMPILER_PATH << " -shared " << RUNTIME_COMPILER_FLAGS << " "
        << tempFile << " -o " << tempSharedObject;
    int rt = std::system(cmd.str().c_str());

    if (rt != 0) {
        throw std::runtime_error("Runtime compilation failed");
    }

    // load library
    void* sharedLib = dlopen(tempSharedObject.c_str(), RTLD_LAZY);
    if (!sharedLib) {
        std::stringstream msg;
        msg << "Cannot open library: " << dlerror() << std::endl;
        throw std::runtime_error(msg.str());
    }

    this->setValueFunction(
            (ValueFunction)dlsym(sharedLib, this->valueFunctionName.c_str()));
    this->setGradientFunction((GradientFunction)dlsym(
            sharedLib, this->gradientFunctionName.c_str()));
    this->setHessianFunction((HessianFunction)dlsym(
            sharedLib, this->hessianFunctionName.c_str()));

    this->setEqualityMatrixFunction((EqualityMatrixFunction)dlsym(
            sharedLib, this->equalityMatrixFunctionName.c_str()));
    this->setEqualityVectorFunction((EqualityVectorFunction)dlsym(
            sharedLib, this->equalityVectorFunctionName.c_str()));

    this->setInequalityValueFunction((InequalityValueFunction)dlsym(
            sharedLib, this->inequalityValueFunctionName.c_str()));
    this->setInequalityGradientFunction((InequalityGradientFunction)dlsym(
            sharedLib, this->inequalityGradientFunctionName.c_str()));
    this->setInequalityHessianFunction((InequalityHessianFunction)dlsym(
            sharedLib, this->inequalityHessianFunctionName.c_str()));

    // Cleanup
    this->_numParameters = this->numParameters();
    this->_numVariables = this->numVariables();
    this->_numEqualityConstraints = this->numEqualityConstraints();
    this->_numInequalityConstraints = this->numInequalityConstraints();
    this->finalized = true;
}

UnorderedSetSymbol SymbolicObjective::getSymbols() const {
    UnorderedSetSymbol allSymbols;

    if (this->objective) {
        util_union(allSymbols, cppmpc::getSymbols(*this->objective));
    }
    util_union(allSymbols, this->equalityConstraints.getSymbols());
    util_union(allSymbols, this->inequalityConstraints.getSymbols());

    return allSymbols;
}

UnorderedSetSymbol SymbolicObjective::getVariables() const {
    UnorderedSetSymbol allVariables;

    if (this->objective) {
        util_union(allVariables, cppmpc::getVariables(*this->objective));
    }
    util_union(allVariables, this->equalityConstraints.getVariables());
    util_union(allVariables, this->inequalityConstraints.getVariables());

    return allVariables;
}

UnorderedSetSymbol SymbolicObjective::getParameters() const {
    UnorderedSetSymbol allParameters;

    if (this->objective) {
        util_union(allParameters, cppmpc::getParameters(*this->objective));
    }
    util_union(allParameters, this->equalityConstraints.getParameters());
    util_union(allParameters, this->inequalityConstraints.getParameters());

    return allParameters;
}

int SymbolicObjective::numParameters() const {
    if (this->_numParameters == -1) {
        return this->getParameters().size();
    } else {
        return this->_numParameters;
    }
}

int SymbolicObjective::numVariables() const {
    if (this->_numVariables == -1) {
        return this->getVariables().size();
    } else {
        return this->_numVariables;
    }
}

int SymbolicObjective::numInequalityConstraints() const {
    if (this->_numInequalityConstraints == -1) {
        return this->inequalityConstraints.numConstraints();
    } else {
        return this->_numInequalityConstraints;
    }
}

int SymbolicObjective::numEqualityConstraints() const {
    if (this->_numEqualityConstraints == -1) {
        return this->equalityConstraints.numConstraints();
    } else {
        return this->_numEqualityConstraints;
    }
}

}  // namespace FastMPC

}  // namespace cppmpc
