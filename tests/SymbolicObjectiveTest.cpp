// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>

#include <symengine/basic.h>
#include <symengine/expression.h>

#include <Eigen/Dense>
#include "FastMPC.h"
#include "OrderedSet.h"
#include "SymEngineUtilities.h"
#include "SymbolicObjective.h"

TEST(SymbolicObjectiveTests, QuadraticSystem) {
    // The Objective
    cppmpc::FastMPC::SymbolicObjective objective;

    // Variables
    SymEngine::Expression x = SymEngine::Expression(cppmpc::variable("x"));
    SymEngine::Expression y = SymEngine::Expression(cppmpc::variable("y"));

    // Parameters
    SymEngine::Expression a = SymEngine::Expression(cppmpc::parameter("a"));

    // Orderings
    cppmpc::OrderedSet variableOrdering;
    variableOrdering.append(x);
    variableOrdering.append(y);
    cppmpc::OrderedSet parameterOrdering;
    parameterOrdering.append(a);

    // Add equality constraints
    objective.equalityConstraints.appendConstraint(x, 3.0);
    objective.inequalityConstraints.appendGreaterThan(y, a);

    // Set the objective
    objective.setObjective(x * x + y * y); 

    // Finalize
    objective.finalize(variableOrdering, parameterOrdering);

    // Set the parameter
    Eigen::VectorXd param(1);
    param << 2.0;
    objective.setParameters(param);

    // Create the solver
    cppmpc::FastMPC::Solver solver = cppmpc::FastMPC::Solver(objective);

    Eigen::VectorXd startPrimal(2);
    startPrimal << 20.0, 20.0;
    auto [minimum, primal, dual] = solver.minimize(startPrimal);

    EXPECT_NEAR(13, minimum, 1e-2);
    EXPECT_NEAR(3, primal(0), 1e-2);
    EXPECT_NEAR(2, primal(1), 1e-2);
}
