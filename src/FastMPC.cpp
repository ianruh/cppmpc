#include "FastMPC.h"

namespace cppmpc {

namespace FastMPC {

std::pair<Eigen::VectorXd, Eigen::VectorXd> Objective::stepSolver(
        const Eigen::VectorXd& gradient, const Eigen::MatrixXd& hessian,
        const Eigen::VectorXd& primal, const Eigen::VectorXd& dual) const {
    std::optional<const Eigen::MatrixXd> equalityConstraintMatrix =
            this->equalityConstraintMatrix();
    std::optional<const Eigen::VectorXd> equalityConstraintVector =
            this->equalityConstraintVector();

    // We split dependning on if there are or aren't equality
    // constraints
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

        // Solve the system. Look at this page for some other
        // options of functions to use:
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

}  // namespace FastMPC

}  // namespace cppmpc
