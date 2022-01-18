// Copyright 2021 Ian Ruh
#include "FastMPC.h"
#include "Util.h"

#include <cmath>
#include <exception>
#include <optional>
#include <tuple>

namespace cppmpc {

namespace FastMPC {

std::pair<Eigen::VectorXd, Eigen::VectorXd> Objective::stepSolver(
        const Eigen::VectorXd& gradient, const Eigen::MatrixXd& hessian,
        const Eigen::VectorXd& primal, const Eigen::VectorXd& dual) const {
    std::optional<const Eigen::MatrixXd> equalityConstraintMatrix =
            this->equalityConstraintMatrix();
    std::optional<const Eigen::VectorXd> equalityConstraintVector =
            this->equalityConstraintVector();

    // We split dependning on if there are or aren't equality constraints
    if (equalityConstraintMatrix.has_value() &&
        equalityConstraintVector.has_value()) {
        // Construct the matrix:
        // ┌         ┐
        // │ ∇²f  Aᵀ │
        // │  A   0  │
        // └         ┘
        // Where A is the matrix for our equality constraints
        Eigen::MatrixXd newtonStepMatrix(
                hessian.rows() + equalityConstraintMatrix->rows(),
                hessian.cols() + equalityConstraintMatrix->rows());
        // Top left
        newtonStepMatrix(Eigen::seqN(0, hessian.rows()),
                         Eigen::seqN(0, hessian.cols())) = hessian;
        // Top right
        newtonStepMatrix(
                Eigen::seqN(0, equalityConstraintMatrix->cols()),
                Eigen::seqN(hessian.cols(), equalityConstraintMatrix->rows())) =
                equalityConstraintMatrix->transpose();
        // Bottom Left
        newtonStepMatrix(
                Eigen::seqN(hessian.rows(), equalityConstraintMatrix->rows()),
                Eigen::seqN(0, equalityConstraintMatrix->cols())) =
                *equalityConstraintMatrix;
        // Bottom Right
        newtonStepMatrix(
                Eigen::seqN(hessian.rows(), equalityConstraintMatrix->rows()),
                Eigen::seqN(hessian.cols(), equalityConstraintMatrix->rows())) =
                Eigen::MatrixXd::Zero(equalityConstraintMatrix->rows(),
                                      equalityConstraintMatrix->rows());

        // Construct the rightside vector
        //  ┌      ┐
        //  │  ∇f  │
        // -│ Ax-b │
        //  └      ┘
        Eigen::VectorXd newtonStepRightSide(gradient.rows() +
                                            equalityConstraintMatrix->rows());
        newtonStepRightSide(Eigen::seqN(0, gradient.rows())) = gradient;
        newtonStepRightSide(Eigen::seqN(gradient.rows(),
                                        equalityConstraintMatrix->rows())) =
                *equalityConstraintMatrix * primal - *equalityConstraintVector;
        newtonStepRightSide *= -1;

        // Solve the system. Look at this page for some other options of
        // functions to use:
        // https://eigen.tuxfamily.org/dox/group__TutorialLinearAlgebra.html
        Eigen::VectorXd stepDirectionWithDual =
                newtonStepMatrix.householderQr().solve(newtonStepRightSide);

        // We need to pull out the step direction from the vector as it
        // includes the dual as well
        // ┌         ┐ ┌     ┐    ┌      ┐
        // │ ∇²f  Aᵀ │ │  v  │    │  ∇f  │
        // │  A   0  │ │  w  │ = -│ Ax-b │
        // └         ┘ └     ┘    └      ┘
        // Where v is our primal step direction, and w would be the next dual
        // (not the dual step)
        Eigen::VectorXd primalStepDirection =
                stepDirectionWithDual(Eigen::seqN(0, this->numVariables()));
        Eigen::VectorXd dualStepDirection =
                stepDirectionWithDual(
                        Eigen::seqN(this->numVariables(),
                                    equalityConstraintVector->rows())) -
                dual;
        // We subtract off the current dual here because w = ν + Δν, while v =
        // Δx

        return std::make_pair(primalStepDirection, dualStepDirection);
    } else {
        // Construct the matrix:
        // ┌     ┐
        // │ ∇²f │
        // └     ┘
        const Eigen::MatrixXd& newtonStepMatrix = hessian;

        // Construct the rightside vector
        //  ┌    ┐
        // -│ ∇f │
        //  └    ┘
        Eigen::VectorXd newtonStepRightSide = -1 * gradient;

        // ┌     ┐ ┌     ┐    ┌      ┐
        // │ ∇²f │ │  v  │ = -│  ∇f  │
        // └     ┘ └     ┘    └      ┘
        // Where v is our primal step direction
        Eigen::VectorXd primalStepDirection =
                newtonStepMatrix.householderQr().solve(newtonStepRightSide);

        return std::make_pair(primalStepDirection,
                              Eigen::VectorXd::Zero(dual.rows()));
    }
}

std::optional<std::string> Objective::validate() const {
    std::optional<const Eigen::MatrixXd> equalityConstraintMatrix =
            this->equalityConstraintMatrix();
    std::optional<const Eigen::VectorXd> equalityConstraintVector =
            this->equalityConstraintVector();
    // Check that if the number of equalioty constraints is greater than 0,
    // then the matrix and vector are returned.
    if (this->numEqualityConstraints() > 0) {
        if (!equalityConstraintMatrix) {
            return "No equality constraint matrix returned even when the "
                   "number of constraints is > 0.";
        }
        if (!equalityConstraintVector) {
            return "No equality constraint vector returned even when the "
                   "number of constraints is > 0.";
        }
    }

    // Check that the equality constraint matix has the same number of columns
    // as the objective has variables
    if (this->numEqualityConstraints() > 0) {
        if (equalityConstraintMatrix->cols() != this->numVariables()) {
            return "Equality constraint matrix has a different number of "
                   "columns than the objective has variables";
        }
    }

    // Check that the equality constraint matrix and vector have the same number
    // of rows
    if (this->numEqualityConstraints() > 0) {
        if (equalityConstraintMatrix->rows() !=
            equalityConstraintVector->rows()) {
            return "Equality constraint matrix and vector have different "
                   "number of rows.";
        }
    }

    // TODO(ianruh): Figure out a good way to check off that other dimensions
    // for the state vector, gradient, and hessian here.

    return std::optional<std::string>();
}

Solver::Solver(const Objective& objective) : objective(objective) {
#ifndef NO_VALIDATE_OBJECTIVE
    // Check the objective dimenions all agree
    std::optional<std::string> objectiveValidationMessage =
            this->objective.validate();
    if (objectiveValidationMessage) {
        throw std::runtime_error(*objectiveValidationMessage);
    }
#endif  // NO_VALIDATE_OBJECTIVE
}

std::tuple<double, Eigen::VectorXd, Eigen::VectorXd> Solver::minimize(
        std::optional<Eigen::VectorXd> primalStart,
        std::optional<Eigen::VectorXd> dualStart) const {
    // Default primal and dual starts
    Eigen::VectorXd currentPoint =
            Eigen::VectorXd::Zero(this->objective.numVariables());
    Eigen::VectorXd currentDual =
            Eigen::VectorXd::Ones(this->objective.numEqualityConstraints());

    if (primalStart) {
        currentPoint = *primalStart;
    }
    if (dualStart) {
        currentDual = *dualStart;
    }

#ifdef NO_VALIDATE_OBJECTIVE
    // Check that the start points are the right dimensions
    if (currentPoint.rows() != objective.numVariables()) {
        std::stringstream msg;
        msg << "Primal start " << currentPoint.format(FlatFmt)
            << " does not have the same number of variables as the objective ("
            << objective.numVariables() << ")";
        throw std::runtime_error(msg.str());
    }
    // Check the dual (empty is no equality constraints)
    if (objective.numEqualityConstraints() > 0 &&
        currentDual.rows() != objective.numEqualityConstraints()) {
        std::stringstream msg;
        msg << "Dual start " << currentDual.format(FlatFmt)
            << " does not have the same number of variables as equality "
               "constraints in the objective ("
            << objective.numEqualityConstraints() << ")";
        throw std::runtime_error(msg.str());
    }
#endif  // VALIDATE_OBJECTIVE

    DEBUG_PRINT("Starting Primal: " << currentPoint.format(FlatFmt));
    DEBUG_PRINT("Starting Dual: " << currentDual.format(FlatFmt));

    // Hyper parameters
    double t = this->hyperParameters.homotopyParameterStart;
    int tSteps = 0;
    int totalSteps = 0;

    double value = objective.value(currentPoint);
    Eigen::VectorXd grad = this->barrierGradient(objective, currentPoint, t);
    Eigen::MatrixXd H = this->barrierHessian(objective, currentPoint, t);
    double lambda = this->residualNorm(objective, currentPoint, currentDual, t);

    bool homotopyStagesExitCondition =
            (objective.numInequalityConstraints() == 0) ||
            (double(objective.numInequalityConstraints()) / t >
             this->hyperParameters.dualGapEpsilon);

    // goto HOMOTOPY_STAGES_LOOP_EXIT is used to break out of this outer loop.
    while (homotopyStagesExitCondition &&
           tSteps < this->hyperParameters.homotopyStagesMaximum &&
           value > this->hyperParameters.valueThreshold) {
        int iterations = 0;

        // This needs to be recalulated because we changed t
        lambda = this->residualNorm(objective, currentPoint, currentDual, t);

        DEBUG_PRINT(tSteps << ":" << iterations
                           << "     Point:   " << currentPoint.format(FlatFmt));
        DEBUG_PRINT(tSteps << ":" << iterations << "     Value:   " << value);
        DEBUG_PRINT(tSteps << ":" << iterations
                           << "     Grad:    " << grad.format(FlatFmt));
        DEBUG_PRINT(tSteps << ":" << iterations << "     Lambda:  " << lambda);

        while (lambda > this->hyperParameters.residualEpsilon &&
               iterations < this->hyperParameters.newtonStepsStageMaximum &&
               value > this->hyperParameters.valueThreshold) {
            // Both of these are Eigen::VectorXd. idk why it needs to be auto
            auto [stepDirectionPrimal, stepDirectionDual] =
                    objective.stepSolver(grad, H, currentPoint, currentDual);

            // TODO: the next point value and residual are calculated twice,
            // once in the line search and again when actually calculating it.
            // This would be a good place for memoization

            // Not really the step length as the newton step direction isn't
            // normalized
            double stepLength = this->infeasibleLinesearch(
                    objective, stepDirectionPrimal, stepDirectionDual,
                    currentPoint, currentDual, t);

            currentPoint = currentPoint + stepLength * stepDirectionPrimal;
            currentDual = currentDual + stepLength * stepDirectionDual;

            iterations += 1;
            totalSteps += 1;

            value = objective.value(currentPoint);
            grad = this->barrierGradient(objective, currentPoint, t);
            H = this->barrierHessian(objective, currentPoint, t);
            lambda =
                    this->residualNorm(objective, currentPoint, currentDual, t);

            DEBUG_PRINT(tSteps << ":" << iterations << "     Point:   "
                               << currentPoint.format(FlatFmt));
            DEBUG_PRINT(tSteps << ":" << iterations
                               << "     Value:   " << value);
            DEBUG_PRINT(tSteps << ":" << iterations
                               << "     Grad:    " << grad.format(FlatFmt));
            DEBUG_PRINT(tSteps << ":" << iterations
                               << "     Lambda:  " << lambda);
        }

        // If we have no inequality constraints, then our first homotopy stage
        // is exact
        if (objective.numInequalityConstraints() == 0) {
            goto HOMOTOPY_STAGES_LOOP_EXIT;
        }

        t *= this->hyperParameters.homotopyParameterMultiplier;
        tSteps += 1;
        homotopyStagesExitCondition =
                (objective.numInequalityConstraints() == 0) ||
                (double(objective.numInequalityConstraints()) / t >
                 this->hyperParameters.dualGapEpsilon);
    }

HOMOTOPY_STAGES_LOOP_EXIT:

    double minimum = objective.value(currentPoint);

    DEBUG_PRINT("t: " << t);
    DEBUG_PRINT("Numer of Iterations: " << totalSteps);
    DEBUG_PRINT("Residual Norm: " << lambda);
    DEBUG_PRINT("Minimum Location: " << currentPoint.format(FlatFmt));
    DEBUG_PRINT("Objective Value: " << minimum);

    return std::make_tuple(minimum, currentPoint, currentDual);
}

double Solver::residualNorm(const Objective& objective,
                            const Eigen::VectorXd& primal,
                            const Eigen::VectorXd& dual, double t) const {
    std::optional<const Eigen::MatrixXd> equalityConstraintMatrix =
            objective.equalityConstraintMatrix();
    std::optional<const Eigen::VectorXd> equalityConstraintVector =
            objective.equalityConstraintVector();

    if (equalityConstraintMatrix && equalityConstraintVector) {
        Eigen::VectorXd vec(equalityConstraintMatrix->cols() +
                            equalityConstraintMatrix->rows());

        // The norm of the full residual shown above
        vec(Eigen::seqN(0, equalityConstraintMatrix->cols())) =
                this->barrierGradient(objective, primal, t) +
                equalityConstraintMatrix->transpose() * dual;
        vec(Eigen::seqN(equalityConstraintMatrix->cols(),
                        equalityConstraintMatrix->rows())) =
                *equalityConstraintMatrix * primal - *equalityConstraintVector;
        return vec.norm();
    } else {
        // If there are no equality constraints, then the residual is just the
        // gradient, so we return the norm of it
        return this->barrierGradient(objective, primal, t).norm();
    }
}

double Solver::infeasibleLinesearch(const Objective& objective,
                                    const Eigen::VectorXd& primalDirection,
                                    const Eigen::VectorXd& dualDirection,
                                    const Eigen::VectorXd& startPrimal,
                                    const Eigen::VectorXd& startDual,
                                    double t) const {
    double s = 1.0;  // Starting line search length

    double shiftedNorm =
            this->residualNorm(objective, startPrimal + s * primalDirection,
                               startDual + s * dualDirection, t);

    const double currentNorm =
            this->residualNorm(objective, startPrimal, startDual, t);

    double shiftedValue =
            this->barrierValue(objective, startPrimal + s * primalDirection, t);

    // We need to make sure we aren't jumping over a barrier
    // e.g. if our barrier is -log(-(0.1 - x)), then the gradient is still
    // defined even when the objective isn't. So, we need to make sure our
    // objective always stays defined (e.g. is not NaN)
    int numIterations = 0;
    while (shiftedNorm > (1 - this->hyperParameters.lineSearchAlpha * s) *
                                 currentNorm ||
           std::isnan(shiftedNorm) || std::isnan(shiftedValue)) {
        s = this->hyperParameters.lineSearchBeta * s;
        shiftedNorm =
                this->residualNorm(objective, startPrimal + s * primalDirection,
                                   startDual + s * dualDirection, t);
        shiftedValue = this->barrierValue(objective,
                                          startPrimal + s * primalDirection, t);
        numIterations += 1;
        if (numIterations > this->hyperParameters.lineSearchMaximumIterations) {
            throw std::runtime_error(
                    "Reached maximum number of line search iterations");
        }
    }
    return s;
}

}  // namespace FastMPC

}  // namespace cppmpc
