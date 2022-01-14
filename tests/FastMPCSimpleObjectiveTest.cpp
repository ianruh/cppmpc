// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>

#include <Eigen/Dense>
#include <optional>
#include <cmath>
#include <iostream>

#include "FastMPC.h"
#include "Util.h"

using namespace cppmpc;

class UnconstrainedQuadraticObjective: public FastMPC::Objective {
 public:
    virtual int numVariables() const override { return 1; }
    virtual int numInequalityConstraints() const override { return 0; }
    virtual int numEqualityConstraints() const override { return 0; }
    
    virtual double value(const Eigen::VectorXd& state) const override {
        return state(0) * state(0);
    }
    virtual const Eigen::VectorXd gradient(const Eigen::VectorXd& state) const override {
        return 2 * state;
    }
    virtual const Eigen::MatrixXd hessian([[maybe_unused]] const Eigen::VectorXd& state) const override {
        return 2.0 * Eigen::MatrixXd::Identity(this->numVariables(), this->numVariables()); 
    }
};

TEST(FastMPCTests, UnconstrainedQuadraticObjective) {
    UnconstrainedQuadraticObjective objective = UnconstrainedQuadraticObjective();

    FastMPC::Solver solver = FastMPC::Solver(objective);

    Eigen::VectorXd startPrimal(1);
    startPrimal << 9.0;
    auto [minimum, primal, dual] = solver.minimize(startPrimal);

    EXPECT_NEAR(0, minimum, 1e-8);
    EXPECT_NEAR(0, primal(0), 1e-8);
}

/**
 * @brief A quadratic objective with additional equality and inequality
 * constraints.
 *
 * Constraints:
 *  - x[0] == 3
 *  - x[1] > 2   --->    -x[1] - 2 < 0
 *
 */
class ConstrainedQuadraticObjective: public FastMPC::Objective {
 public:
    virtual int numVariables() const override { return 2; }
    virtual int numInequalityConstraints() const override { return 1; }
    virtual int numEqualityConstraints() const override { return 1; }
    
    virtual double value(const Eigen::VectorXd& state) const override {
        return state.dot(state);
    }
    virtual const Eigen::VectorXd gradient(const Eigen::VectorXd& state) const override {
        return 2 * state;
    }
    virtual const Eigen::MatrixXd hessian([[maybe_unused]] const Eigen::VectorXd& state) const override {
        return 2.0 * Eigen::MatrixXd::Identity(this->numVariables(), this->numVariables()); 
    }

    virtual std::optional<const Eigen::MatrixXd> equalityConstraintMatrix() const override {
        Eigen::MatrixXd mat(1, 2);
        mat << 1.0, 0.0;
        return mat;
    }
    virtual std::optional<const Eigen::VectorXd> equalityConstraintVector() const override {
        Eigen::VectorXd vec(1);
        vec << 3.0;
        return vec;
    }

    virtual double inequalityConstraintsValue(const Eigen::VectorXd& state) const override {
        return -1*std::log((state(1) - 2.0));
    }
    virtual const Eigen::VectorXd inequalityConstraintsGradient(const Eigen::VectorXd& state) const override {
        Eigen::VectorXd grad(2);
        grad << 0.0, -1.0/(state(1) - 2.0);
        return grad;
    }
    virtual const Eigen::MatrixXd inequalityConstraintsHessian(const Eigen::VectorXd& state) const override {
        Eigen::MatrixXd mat(2,2);
        mat << 0.0, 0.0, 0.0, 1.0/std::pow((state(1) - 2.0),2);
        return mat;
    }
};

TEST(FastMPCTests, ConstrainedQuadraticObjective) {
    ConstrainedQuadraticObjective objective = ConstrainedQuadraticObjective();

    FastMPC::Solver solver = FastMPC::Solver(objective);

    Eigen::VectorXd startPrimal(2);
    startPrimal << 20.0, 20.0;
    auto [minimum, primal, dual] = solver.minimize(startPrimal);

    EXPECT_NEAR(13, minimum, 1e-2);
    EXPECT_NEAR(3, primal(0), 1e-2);
    EXPECT_NEAR(2, primal(1), 1e-2);
}
