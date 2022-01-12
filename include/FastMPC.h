#ifndef INCLUDE_FASTMPC_H_
#define INCLUDE_FASTMPC_H_

#include <limits>
#include <optional>
#include <tuple>

#include <Eigen/Dense>

// TODO(ianruh): Init checks
// TODO(ianruh): Core loop functions
// TODO(ianruh): Residual  norm
// TODO(ianruh): Infasible line search
// TODO(ianruh): Barrier value
// TODO(ianruh): Barrier gradient
// TODO(ianruh): Barrier Hessian

namespace cppmpc {

namespace FastMPC {

class Objective {
 public:
    /// Number of variables taken by the objective
    virtual int numVariables() const = 0;

    /// Number of inequality constraints
    virtual int numConstraints() const = 0;

    //==================== Objective =================

    /// The value of the objective at a given point
    ///
    /// - Parameter x: The point to evaluate the objective at
    /// - Returns: The value of teh objective
    virtual double value(const Eigen::VectorXd& state) const = 0;

    /// The value of the gradient at a given point
    ///
    /// - Parameter x: The point to evaulate the gradient at
    /// - Returns: The value of teh gradient
    virtual const Eigen::VectorXd gradient(
            const Eigen::VectorXd& state) const = 0;

    /// The value of the Hessian at a given point.
    ///
    /// - Parameter x: The point to evaluate the Hessian at.
    /// - Returns: The value of the Hessian.
    virtual const Eigen::MatrixXd hessian(
            const Eigen::VectorXd& state) const = 0;

    //================= Equality ================

    /// The equality constraint matrix. If the equality constraints are
    /// non-linear, this has to be the linearized equality constraints (possibly
    /// about the current/previous state).
    virtual std::optional<const Eigen::MatrixXd> equalityConstraintMatrix()
            const = 0;

    /// The reuality constraint vector (the right hand side of  Ax=b).
    virtual std::optional<const Eigen::VectorXd> equalityConstraintVector()
            const = 0;

    //=========== Inequality Constraints ============

    /**
     * @brief The sum of `-1*log(-1*constraint)` of the inequality
     * constraints in normal form  (e.g. <= 0). Other barrier
     * functions should work, but have not been tried.
     *
     * The default implementation returns 0 as a default in the
     * case of no inequality constraints.
     *
     * @param state The current primal state.
     * @return The value of the sum of the negative logs.
     */
    virtual double inequalityConstraintsValue([
            [maybe_unused]] const Eigen::VectorXd& state) const {
        return 0.0;
    }

    /**
     * @brief The value of the gradient of the inequality
     * constraints value.
     *
     * The default implementation returns a vector 0 zeros with the
     * same number of elements as the number of variables, as
     * would be expected in the case without inequality constraints.
     *
     * @param Vector The current primal state.
     * @return The gradient of the inequalityConstraintsValue.
     */
    virtual const Eigen::VectorXd inequalityConstraintsGradient([
            [maybe_unused]] const Eigen::VectorXd& state) const {
        return Eigen::VectorXd::Zero(this->numVariables());
    }

    /**
     * @brief The value of the Hessian of the inequality constraints.
     *
     * @param Vector The current primal state.
     * @return The Hessian of the inequalityConstraintsValue
     */
    virtual const Eigen::MatrixXd inequalityConstraintsHessian([
            [maybe_unused]] const Eigen::VectorXd& state) const {
        return Eigen::MatrixXd::Zero(this->numVariables(),
                                     this->numVariables());
    }

    //================= Step Solver Default Implementation ================

    /// Step Solver Default Implementation
    ///
    /// TODO(ianruh): This doesn't handle the case of singular H (it will
    /// explode in some random direction)
    ///
    /// [Here are some
    /// options](https://math.stackexchange.com/questions/2092999/a-question-about-newtons-method-for-equality-constrained-convex-minimization)
    ///
    /// It shouldn't come up if our objective is strongly convex, as then we are
    /// gaurenteed the hessian will be non-singular. However, for the problem of
    /// finding a feasible point, we  minimize the scalar function s, which is
    /// clearly not convex (it doesn't even have a minimum). We adapted the
    /// solver to stop when s became negative, but it will explode before then.
    /// In stead, we add a constraint to that problem that s >= -10. This
    /// *shouldn't* restrict any feasible objectives, and will gaurentee that
    /// our barrier augmented objective is non-singular.
    ///
    /// - Parameters:
    ///   - gradient: The current gradient of the objective.
    ///   - hessian: The current hessian of the objective.
    ///   - primal: The current primal.
    ///   - dual: The current dual.
    /// - Throws: If some part of the objective cannot be evaulated or the
    /// resulting system is infeasible.
    /// - Returns: The primal and dual newton steps.
    virtual std::pair<Eigen::VectorXd, Eigen::VectorXd> stepSolver(
            const Eigen::VectorXd& gradient, const Eigen::MatrixXd& hessian,
            const Eigen::VectorXd& primal, const Eigen::VectorXd& dual) const;
};

class Solver {
 public:
    Solver();
};

/**
 * struct HyperParameters - Just a container for the hyper paramaters
 * that can be used to customize the solved.
 *
 * The default values are firly aggressive, so will find a pretty
 * precise solution. To improve runtime performance, the iteration
 * maximums can be heavily restricted.
 */
struct HyperParameters {
    //==== Iteration Maximums ====

    /**
     * @newtonStepsStageMaximum - The maximum number of newton steps
     * per homotopy stage.
     */
    int newtonStepsStageMaximum = 100;

    /// The maximum number of homotopy stages to to perform.
    int homotopyStagesMaximum = 50;

    //==== Epsilons ====

    /// The epsilon value used for the residual.
    double residualEpsilon = 1.0e-3;

    /// The epsilon value used for the primal-dual gap.
    double dualGapEpsilon = 1.0e-3;

    //==== Homtopy Parameters ====

    /// The starting value of the homotopy barrier parameter.
    double homotopyParameterStart = 1.0;

    /// The multiplier for the homotopy barrier parameter. It is the factor that
    /// the parameter increases by after every stage.
    double homotopyParameterMultiplier = 20.0;

    //==== Line Search ====
    /// Back tracking line search alpha parameter
    /// [reference](https://en.wikipedia.org/wiki/Backtracking_line_search
    double lineSearchAlpha = 0.25;
    /// Back tracking line  search beta  parameter
    /// [reference](https://en.wikipedia.org/wiki/Backtracking_line_search)
    double lineSearchBeta = 0.5;
    /// The maximum number  of line search iterations.
    int lineSearchMaximumIterations = 100;

    //==== Misc ====
    /// A threshold for the value of the objective, If this value is achieved,
    /// then the solver returns. By default, the value is -inf, so has no effect
    /// on the solver.
    double valueThreshold = -1 * std::numeric_limits<double>::infinity();
};

}  // namespace FastMPC

}  // namespace cppmpc

#endif  // INCLUDE_FASTMPC_H_
