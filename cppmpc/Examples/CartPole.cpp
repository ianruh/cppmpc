// Copyright 2021 Ian Ruh
#include "FastMPC.h"
#include "OrderedSet.h"
#include "SymEngineUtilities.h"
#include "SymbolicObjective.h"

#include <symengine/basic.h>
#include <symengine/expression.h>
#include <symengine/functions.h>
#include <symengine/pow.h>
#include <Eigen/Dense>
#include <pybind11/embed.h>

#include <iostream>
#include <memory>
#include <vector>

namespace py = pybind11;

using cppmpc::sin;
using cppmpc::cos;
using cppmpc::tan;
using cppmpc::pow;
using cppmpc::taylorExpand;
using SymEngine::integer;

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    // Some problem parameters
    size_t numSteps = 20;
    double maxPos = 10.0;
    double minPos = -10.0;
    double maxForce = 10.0;
    double minForce = -10.0;
    double dt = 0.1;

    // Build the objective
    cppmpc::FastMPC::SymbolicObjective objective;

    //===== State Variables =====
    // These are numSteps length vectors with each element representing points in time
    std::vector<SymEngine::Expression> position = cppmpc::toExpressions(cppmpc::variableVector("x", numSteps));
    std::vector<SymEngine::Expression> velocity = cppmpc::toExpressions(cppmpc::variableVector("v", numSteps));
    std::vector<SymEngine::Expression> angle = cppmpc::toExpressions(cppmpc::variableVector("theta", numSteps));
    std::vector<SymEngine::Expression> angularVelocity = cppmpc::toExpressions(cppmpc::variableVector("thetad", numSteps));

    //===== Control Variables ======
    std::vector<SymEngine::Expression> force = cppmpc::toExpressions(cppmpc::variableVector("f", numSteps));

    //===== Variable Ordering =====
    cppmpc::OrderedSet variableOrdering;
    variableOrdering.append(SymEngine::Expression(cppmpc::variable("test")));
    for(size_t i = 0; i < numSteps; i++) {
        variableOrdering.append(position[i]);
        variableOrdering.append(velocity[i]);
        variableOrdering.append(angle[i]);
        variableOrdering.append(angularVelocity[i]);
        variableOrdering.append(force[i]);
    }

    //===== Parameters ======

    // Initial position parameters
    SymEngine::Expression initialPosition = SymEngine::Expression(cppmpc::parameter("x0"));
    SymEngine::Expression initialVelocity = SymEngine::Expression(cppmpc::parameter("v0"));
    SymEngine::Expression initialAngle = SymEngine::Expression(cppmpc::parameter("theta0"));
    SymEngine::Expression initialAngularVelocity = SymEngine::Expression(cppmpc::parameter("thetad0"));

    // Problem parameters
    SymEngine::Expression poleLength = SymEngine::Expression(cppmpc::parameter("poleLength"));
    SymEngine::Expression cartMass = SymEngine::Expression(cppmpc::parameter("cartMass"));
    SymEngine::Expression poleMass = SymEngine::Expression(cppmpc::parameter("poleMass"));
    SymEngine::Expression gravity = SymEngine::Expression(cppmpc::parameter("gravity"));
    
    // Previous iteration state and control variables. These are used to linearize
    // the dynamics around approximately the correct points.
    std::vector<SymEngine::Expression> previousPosition = cppmpc::toExpressions(cppmpc::parameterVector("x_prev", numSteps));
    std::vector<SymEngine::Expression> previousVelocity = cppmpc::toExpressions(cppmpc::parameterVector("v_prev", numSteps));
    std::vector<SymEngine::Expression> previousAngle = cppmpc::toExpressions(cppmpc::parameterVector("theta_prev", numSteps));
    std::vector<SymEngine::Expression> previousAngularVelocity = cppmpc::toExpressions(cppmpc::parameterVector("thetad_prev", numSteps));

    //====== Parameter Ordering ======
    cppmpc::OrderedSet parameterOrdering;
    parameterOrdering.append(initialPosition);
    parameterOrdering.append(initialVelocity);
    parameterOrdering.append(initialAngle);
    parameterOrdering.append(initialAngularVelocity);
    parameterOrdering.append(poleLength);
    parameterOrdering.append(cartMass);
    parameterOrdering.append(poleMass);
    parameterOrdering.append(gravity);

    for(size_t i = 0; i < numSteps; i++) {
        parameterOrdering.append(previousPosition[i]);
        parameterOrdering.append(previousVelocity[i]);
        parameterOrdering.append(previousAngle[i]);
        parameterOrdering.append(previousAngularVelocity[i]);
    }

    //===== Constraints =====
    
    // Initial conditions
    objective.equalityConstraints.appendConstraint(position[0], initialPosition);
    objective.equalityConstraints.appendConstraint(velocity[0], initialVelocity);
    objective.equalityConstraints.appendConstraint(angle[0], initialAngle);
    objective.equalityConstraints.appendConstraint(angularVelocity[0], initialAngularVelocity);

    // Set the maximum position and maximum force.
    for(size_t i = 0; i < numSteps; i++) {
        objective.inequalityConstraints.appendLessThan(position[i], maxPos);
        objective.inequalityConstraints.appendGreaterThan(position[i], minPos);
        objective.inequalityConstraints.appendLessThan(force[i], maxForce);
        objective.inequalityConstraints.appendGreaterThan(force[i], minForce);
    }

    // Dynamics constraints
    for(size_t t = 1; t < numSteps; t++) {
        // Position constraint
        objective.equalityConstraints.appendConstraint(position[t], position[t-1] + 0.5*dt*velocity[t-1] + 0.5*dt*velocity[t]);
        
        // Velocity Constraint
        SymEngine::Expression k = 4.0/3.0;
        SymEngine::Expression linear_acceleration_1 = gravity*taylorExpand(sin(angle[t-1])*cos(angle[t-1])/(pow(cos(angle[t-1]), integer(2)) - k*(cartMass/poleMass + 1.0)), angle[t-1], previousAngle[t], 1);
        SymEngine::Expression linear_acceleration_2 = k*force[t-1]/(poleMass*pow(cos(previousAngle[t]), integer(2)) - k*(cartMass + poleMass));
        SymEngine::Expression linear_acceleration_3 = k*poleLength*sin(previousAngle[t])/(pow(cos(previousAngle[t]), integer(2)) - k*(cartMass/poleMass + 1)) * taylorExpand(pow(angularVelocity[t-1], integer(2)), angularVelocity[t-1], previousAngularVelocity[t], 1);
        
        objective.equalityConstraints.appendConstraint(velocity[t], velocity[t-1] + dt*linear_acceleration_1 - dt*linear_acceleration_2 - dt*linear_acceleration_3);
        
        // Angle Constraint
        objective.equalityConstraints.appendConstraint(angle[t], angle[t-1] + 0.5*dt*angularVelocity[t-1] + 0.5*dt*angularVelocity[t]);
        
        // Angular Velocity Constraint
        SymEngine::Expression angular_acceleration_1 = gravity*(cartMass + poleMass) * taylorExpand(sin(angle[t-1])/(k*(cartMass+poleMass)*poleLength - poleMass*poleLength*pow(cos(angle[t-1]), integer(2))), angle[t-1], previousAngle[t], 1);
        SymEngine::Expression angular_acceleration_2 = force[t-1]/(k*(cartMass + poleMass)*poleLength - poleMass*poleLength*cos(previousAngle[t]));
        SymEngine::Expression angular_acceleration_3 = sin(previousAngle[t])/(k*(cartMass/poleMass + 1) - cos(previousAngle[t])) * taylorExpand(pow(angularVelocity[t-1], integer(2)), angularVelocity[t-1], previousAngularVelocity[t], 1);
        
        objective.equalityConstraints.appendConstraint(angularVelocity[t], angularVelocity[t-1] + dt*angular_acceleration_1 - dt*angular_acceleration_2 - dt*angular_acceleration_3);
        
    }

    // Set the Objective

    //====== Finalize the Objective ======
    objective.finalize(variableOrdering, parameterOrdering);

    // Now that the objective is finalized, we can set the initial parameter
    // values.

    // Start the python interpreter
    py::scoped_interpreter guard{};

    py::print("Hello, World!");
    return 0;
}
