// Copyright 2021 Ian Ruh
#include "FastMPCFunctionPointerObjective.h"

#include <Eigen/Dense>
#include <optional>
#include <string>

namespace cppmpc {

namespace FastMPC {

void FunctionPointerObjective::DefaultFunctions::equalityMatrixFunction(
        [[maybe_unused]] const double* param, [[maybe_unused]] double* out) {}

void FunctionPointerObjective::DefaultFunctions::equalityVectorFunction(
        [[maybe_unused]] const double* param, [[maybe_unused]] double* out) {}

void FunctionPointerObjective::DefaultFunctions::inequalityValueFunction(
        [[maybe_unused]] const double* state,
        [[maybe_unused]] const double* param, [[maybe_unused]] double* out) {}

void FunctionPointerObjective::DefaultFunctions::inequalityGradientFunction(
        [[maybe_unused]] const double* state,
        [[maybe_unused]] const double* param, [[maybe_unused]] double* out) {}

void FunctionPointerObjective::DefaultFunctions::inequalityHessianFunction(
        [[maybe_unused]] const double* state,
        [[maybe_unused]] const double* param, [[maybe_unused]] double* out) {}

std::optional<std::string> FunctionPointerObjective::validate() const {
    // Check that each function pointer is non-null.
    if (this->valueFunction == nullptr) {
        return "Value function pointer is null";
    }

    if (this->gradientFunction == nullptr) {
        return "Gradient function pointer is null";
    }

    if (this->hessianFunction == nullptr) {
        return "Hessian function pointer is null";
    }

    if (this->equalityMatrixFunction == nullptr) {
        return "Equality matrix function pointer is null";
    }

    if (this->equalityVectorFunction == nullptr) {
        return "Equality vector function pointer is null";
    }

    if (this->inequalityValueFunction == nullptr) {
        return "Inequality value function pointer is null";
    }

    if (this->inequalityGradientFunction == nullptr) {
        return "Inequality gradient function pointer is null";
    }

    if (this->inequalityHessianFunction == nullptr) {
        return "Inequality hessian function pointer is null";
    }

    // Check that if we have parameters, then the parameters vector exists and
    // is the right size.
    if (this->numParameters() > 0 && !this->_parameters &&
        this->_parameters->rows() == this->numParameters()) {
        return "Number of parameters does not match the parameters vector "
               "size, or the vector has not been set";
    }

    // Run the original validation checks now.
    return Objective::validate();
}

FunctionPointerObjective::FunctionPointerObjective(int numVariables,
                                                   int numInequalityConstraints,
                                                   int numEqualityConstraints,
                                                   int numParameters)
        : _numVariables(numVariables),
          _numInequalityConstraints(numInequalityConstraints),
          _numEqualityConstraints(numEqualityConstraints),
          _numParameters(numParameters) {}

void FunctionPointerObjective::setParameters(Eigen::VectorXd parameters) {
    this->_parameters = parameters;
}

double FunctionPointerObjective::value(const Eigen::VectorXd& state) const {
    double value;
    (*this->valueFunction)(state.data(), this->_parameters->data(), &value);
    return value;
}

void FunctionPointerObjective::setValueFunction(ValueFunction functionPtr) {
    this->valueFunction = functionPtr;
}

const Eigen::VectorXd FunctionPointerObjective::gradient(
        const Eigen::VectorXd& state) const {
    Eigen::VectorXd vec(this->numVariables());
    (*this->gradientFunction)(state.data(), this->_parameters->data(),
                              vec.data());
    return vec;
}

void FunctionPointerObjective::setGradientFunction(
        GradientFunction functionPtr) {
    this->gradientFunction = functionPtr;
}

const Eigen::MatrixXd FunctionPointerObjective::hessian(
        const Eigen::VectorXd& state) const {
    Eigen::MatrixXd mat(this->numVariables(), this->numVariables());
    (*this->hessianFunction)(state.data(), this->_parameters->data(),
                             mat.data());
    return mat;
}

void FunctionPointerObjective::setHessianFunction(HessianFunction functionPtr) {
    this->hessianFunction = functionPtr;
}

std::optional<const Eigen::MatrixXd>
FunctionPointerObjective::equalityConstraintMatrix() const {
    if (this->numEqualityConstraints() > 0) {
        Eigen::MatrixXd mat(this->numEqualityConstraints(),
                            this->numVariables());
        (*this->equalityMatrixFunction)(this->_parameters->data(), mat.data());
        return mat;
    } else {
        return std::optional<const Eigen::MatrixXd>();
    }
}

void FunctionPointerObjective::setEqualityMatrixFunction(
        EqualityMatrixFunction functionPtr) {
    this->equalityMatrixFunction = functionPtr;
}

std::optional<const Eigen::VectorXd>
FunctionPointerObjective::equalityConstraintVector() const {
    if (this->numEqualityConstraints() > 0) {
        Eigen::VectorXd vec(this->numEqualityConstraints());
        (*this->equalityVectorFunction)(this->_parameters->data(), vec.data());
        return vec;
    } else {
        return std::optional<const Eigen::VectorXd>();
    }
}

void FunctionPointerObjective::setEqualityVectorFunction(
        EqualityVectorFunction functionPtr) {
    this->equalityVectorFunction = functionPtr;
}

double FunctionPointerObjective::inequalityConstraintsValue(
        const Eigen::VectorXd& state) const {
    if (this->numInequalityConstraints() > 0) {
        double value;
        (*this->inequalityValueFunction)(state.data(),
                                         this->_parameters->data(), &value);
        return value;
    } else {
        return Objective::inequalityConstraintsValue(state);
    }
}

void FunctionPointerObjective::setInequalityValueFunction(
        InequalityValueFunction functionPtr) {
    this->inequalityValueFunction = functionPtr;
}

const Eigen::VectorXd FunctionPointerObjective::inequalityConstraintsGradient(
        const Eigen::VectorXd& state) const {
    if (this->numInequalityConstraints() > 0) {
        Eigen::VectorXd vec(this->numVariables());
        (*this->inequalityGradientFunction)(
                state.data(), this->_parameters->data(), vec.data());
        return vec;
    } else {
        return Objective::inequalityConstraintsGradient(state);
    }
}

void FunctionPointerObjective::setInequalityGradientFunction(
        InequalityGradientFunction functionPtr) {
    this->inequalityGradientFunction = functionPtr;
}

const Eigen::MatrixXd FunctionPointerObjective::inequalityConstraintsHessian(
        const Eigen::VectorXd& state) const {
    if (this->numInequalityConstraints() > 0) {
        Eigen::MatrixXd mat(this->numVariables(), this->numVariables());
        (*this->inequalityHessianFunction)(
                state.data(), this->_parameters->data(), mat.data());
        return mat;
    } else {
        return Objective::inequalityConstraintsHessian(state);
    }
}

void FunctionPointerObjective::setInequalityHessianFunction(
        InequalityHessianFunction functionPtr) {
    this->inequalityHessianFunction = functionPtr;
}

}  // namespace FastMPC

}  // namespace cppmpc
