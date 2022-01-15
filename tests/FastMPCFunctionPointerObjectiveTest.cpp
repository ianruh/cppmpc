// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>

#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <optional>

#include "FastMPC.h"
#include "Util.h"
#include "FastMPCFunctionPointerObjective.h"

using namespace cppmpc;

/**
 * @brief A quadratic objective with additional equality and inequality
 * constraints implemented using the function pointer objective. Refer to the
 * simple objective test for a non-function pointer implemnetation of the
 * same objective.
 *
 * Num Variables: 2
 * Num Equality Constraints: 1
 * Num Inequality Constraints: 1
 * Num Parameters: 1
 *
 * Constraints:
 *  - x[0] == param[0]
 *  - x[1] > 2   --->    -x[1] - 2 < 0
 *
 */
//=============================================================================
void valueFunction(
        const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    *out = state[0] * state[0] + state[1] * state[1];
}

void gradientFunction(
        const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    out[0] = 2*state[0];
    out[1] = 2*state[1];
}


void hessianFunction(
        [[maybe_unused]] const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    // The output matrix is column major
    out[0] = 2.0; // (0,0)
    out[1] = 0.0; // (1,0)
    out[2] = 0.0; // (0,1)
    out[3] = 2.0; // (1,1)
}

void equalityMatrixFunction(
        [[maybe_unused]] const double* param,
        double* out) {
    out[0] = 1.0;
    out[1] = 0.0;
}

void equalityVectorFunction(
        [[maybe_unused]] const double* param,
        double* out) {
    out[0] = param[0];
}

void inequalityValueFunction(
        const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    *out = -1 * std::log((state[1] - 2.0));
}

void inequalityGradientFunction(
        const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    out[0] = 0.0;
    out[1] = -1.0 / (state[1] - 2.0);
}

void inequalityHessianFunction(
        const double* state,
        [[maybe_unused]] const double* param,
        double* out) {
    out[0] = 0.0; // (0,0)
    out[1] = 0.0; // (1,0)
    out[2] = 0.0; // (0,1)
    out[3] = 1.0 / std::pow((state[1] - 2.0), 2); // (1,1)
}
//=============================================================================

TEST(FastMPCTests, FunctionPointerObjective) {
    FastMPC::FunctionPointerObjective objective = FastMPC::FunctionPointerObjective(
            2, 1, 1, 1);

    objective.setValueFunction(&valueFunction);
    objective.setGradientFunction(&gradientFunction);
    objective.setHessianFunction(&hessianFunction);
    objective.setEqualityMatrixFunction(&equalityMatrixFunction);
    objective.setEqualityVectorFunction(&equalityVectorFunction);
    objective.setInequalityValueFunction(&inequalityValueFunction);
    objective.setInequalityGradientFunction(&inequalityGradientFunction);
    objective.setInequalityHessianFunction(&inequalityHessianFunction);

    Eigen::VectorXd param(1);
    param << 3.0;
    objective.setParameters(param);

    FastMPC::Solver solver = FastMPC::Solver(objective);

    Eigen::VectorXd startPrimal(2);
    startPrimal << 20.0, 20.0;
    auto [minimum, primal, dual] = solver.minimize(startPrimal);

    EXPECT_NEAR(13, minimum, 1e-2);
    EXPECT_NEAR(3, primal(0), 1e-2);
    EXPECT_NEAR(2, primal(1), 1e-2);
}
