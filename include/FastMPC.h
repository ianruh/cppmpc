#ifndef INCLUDE_FASTMPC_H_
#define INCLUDE_FASTMPC_H_

#include <limits>
#include <optional>
#include <tuple>

#include <Eigen/Dense>

namespace cppmpc {

namespace FastMPC {

/**
 * struct HyperParameters - Just a container for the hyper paramaters
 * that can be used to customize the solved.
 *
 * The default values are firly aggressive, so will find a pretty
 * precise solution. To improve runtime performance, the iteration
 * maximums can be heavily restricted.
 */
typedef struct HyperParameters {
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
} HyperParameters;

class Objective {
 public:
    /**
     * @brief Number of variables taken by the objective.
     *
     * This is called often in solving the function and should be efficient (
     * e.g should not construct a matrix and check its dimensions every time).
     */
    virtual int numVariables() const = 0;

    /**
     * @brief Number of inequality constraints.
     *
     * This is called often in solving the function and should be efficient (
     * e.g should not construct a matrix and check its dimensions every time).
     */
    virtual int numInequalityConstraints() const = 0;

    /**
     * @brief Number of equality constraints.
     *
     * This is called often in solving the function and should be efficient (
     * e.g should not construct a matrix and check its dimensions every time).
     */
    virtual int numEqualityConstraints() const = 0;

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
            const {
        return std::optional<const Eigen::MatrixXd>();
    }

    /// The reuality constraint vector (the right hand side of  Ax=b).
    virtual std::optional<const Eigen::VectorXd> equalityConstraintVector()
            const {
        return std::optional<const Eigen::VectorXd>();
    }

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

    /**
     * @brief Validate that the dimensions of the objective all agree and the
     * reported number of variables and constraints is correct.
     *
     */
    virtual std::optional<std::string> validate() const;
};

class Solver {
 private:
    // TODO(ianruh): Get and set hyper parameters
    HyperParameters hyperParameters;

    const Objective& objective;

 public:
    /**
     * @brief Validate that the given objective is well formed.
     *
     * @param objective The objective that will be optimized.
     */
    Solver(const Objective& objective);

    /// Minimize an infeasible start, inequality constrained objective.
    /// - Parameter objective: The objective to minimize.
    /// - Throws: For many reasons, including (ill formed problems, evaluation
    /// problems, line search problems, and others).
    /// - Returns: The minimum objective value, the minimum's primal, and the
    /// minimum's dual).
    ///
    /// The given primal should be strictly feasible, meaning it obeys all the
    /// inequality constraints, but may violate the equality constraints.
    std::tuple<double, Eigen::VectorXd, Eigen::VectorXd> minimize(
            std::optional<Eigen::VectorXd> primalStart =
                    std::optional<Eigen::VectorXd>(),
            std::optional<Eigen::VectorXd> dualStart =
                    std::optional<Eigen::VectorXd>()) const;

    /// The norm of
    ///
    /// ```
    /// ┌          ┐
    /// │ ∇f + Aᵀν │
    /// │  Ax - b  │
    /// └          ┘
    /// ```
    ///
    /// - Parameters:
    ///   - objective: The objective veing minimized.
    ///   - primal: The current primal value.
    ///   - dual: The current dual value
    ///   - t: The current barrier parameter.
    /// - Returns: The residual  of the norm.
    double residualNorm(const Objective& objective,
                        const Eigen::VectorXd& primal,
                        const Eigen::VectorXd& dual, double t) const;

    /// Perform an infeasible start line search on the problem.
    ///
    /// If an exception is thrown saying that the maximum number of line search
    /// iterations has been reached, That usually means the current  primal/dual
    /// is infeasible, so it can't progress in any direction.
    ///
    /// - Parameters:
    ///   - objective: The objective being minimized.
    ///   - primalDirection: The primal direction to search in.
    ///   - dualDirection: The dual direction to seach in.
    ///   - startPrimal: The starting primal values.
    ///   - startDual: The starting dual value.
    ///   - t: The current barrier parameter.
    /// - Throws: If the maximum number of line search iterations has been hit.
    /// - Returns: The step length found.
    double infeasibleLinesearch(const Objective& objective,
                                const Eigen::VectorXd& primalDirection,
                                const Eigen::VectorXd& dualDirection,
                                const Eigen::VectorXd& startPrimal,
                                const Eigen::VectorXd& startDual,
                                double t) const;

    /// The value of the barrier augmented objective.
    /// - Parameters:
    ///   - objective: The objective being minimized.
    ///   - x: The current primal values.
    ///   - t: The current barrier parameter.
    /// - Returns: The augmented objective value.
    double barrierValue(const Objective& objective,
                        const Eigen::VectorXd& state, double t) const {
        return t * objective.value(state) +
               objective.inequalityConstraintsValue(state);
    }

    /// The value of the barrier augmented objective's gradient.
    /// - Parameters:
    ///   - objective: The objective being minimized.
    ///   - x: The current primal value.
    ///   - t: The current barrier parameter.
    /// - Returns: The augmented objective's gradient.
    Eigen::VectorXd barrierGradient(const Objective& objective,
                                    const Eigen::VectorXd& state,
                                    double t) const {
        return t * objective.gradient(state) +
               objective.inequalityConstraintsGradient(state);
    }

    /// The value  of the augmented objective's hessian.
    /// - Parameters:
    ///   - objective: The objective being minimized.
    ///   - x: The cureent primal value.
    ///   - t: The current barrier parameter.
    /// - Returns: The augmented  objective's hessian.
    Eigen::MatrixXd barrierHessian(const Objective& objective,
                                   const Eigen::VectorXd& state,
                                   double t) const {
        return t * objective.hessian(state) +
               objective.inequalityConstraintsHessian(state);
    }
};

}  // namespace FastMPC

}  // namespace cppmpc

#endif  // INCLUDE_FASTMPC_H_
