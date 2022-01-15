#ifndef INCLUDE_FASTMPCFUNCTIONPOINTEROBJECTIVE_H_
#define INCLUDE_FASTMPCFUNCTIONPOINTEROBJECTIVE_H_

#include "FastMPC.h"
#include "Util.h"
#include <Eigen/Dense>
#include <optional>
#include <string>

namespace cppmpc {

namespace FastMPC {

// TODO(ianruh): Reuse the same vector/matrix for each of the functions each
// time they are called rather than allocating a new one.
/**
 * @brief An objective that acts as a wrapper for a set of C function pointers.
 *
 * Each of  the function pointers corresponds to one of the pure virtual
 * functions in FastMPC::Objective. Refer to those functions for what each
 * function should calculate, and refer to the set* functions in this class
 * for the semantics of accessing and returning the values/vectors/matrices.
 */
class FunctionPointerObjective: public Objective {
 private:
    // Internal storage for the number of variables
    int _numVariables;
    // Internal storage for the number of inequality constraints
    int _numInequalityConstraints;
    // Internal storage for the number of equality constraints
    int _numEqualityConstraints;

    // Storing the parameters set for the objective by the user
    int _numParameters;
    std::optional<Eigen::VectorXd> _parameters;

 public:
    // Types of the function pointers being used
    typedef void (*ValueFunction)(const double* state, const double* param, double* out);
    typedef void (*GradientFunction)(const double* state, const double* param, double* out);
    typedef void (*HessianFunction)(const double* state, const double* paramm, double* out);

    typedef void (*EqualityMatrixFunction)(const double* param, double* out);
    typedef void (*EqualityVectorFunction)(const double* param, double* out);

    typedef void (*InequalityValueFunction)(const double* state, const double* param, double* out);
    typedef void (*InequalityGradientFunction)(const double* state, const double* param, double* out);
    typedef void (*InequalityHessianFunction)(const double* state, const double* param, double* out);

 private:
    /**
     * struct DefaultFunctions - Default implementations of the functions to
     * function pointers of.
     */
    typedef struct DefaultFunctions {
        static void equalityMatrixFunction(const double* param, double* out);
        static void equalityVectorFunction(const double* param, double* out);

        static void inequalityValueFunction(const double* state, const double* param, double* out);
        static void inequalityGradientFunction(const double* state, const double* param, double* out);
        static void inequalityHessianFunction(const double* state, const double* param, double* out);
    } DefaultFunctions;
    
    //============= Function pointer storage =============
    ValueFunction valueFunction = nullptr;
    GradientFunction gradientFunction = nullptr;
    HessianFunction hessianFunction = nullptr;

    EqualityMatrixFunction equalityMatrixFunction = &DefaultFunctions::equalityMatrixFunction;
    EqualityVectorFunction equalityVectorFunction = &DefaultFunctions::equalityVectorFunction;

    InequalityValueFunction inequalityValueFunction = &DefaultFunctions::inequalityValueFunction;
    InequalityGradientFunction inequalityGradientFunction = &DefaultFunctions::inequalityGradientFunction;
    InequalityHessianFunction inequalityHessianFunction = &DefaultFunctions::inequalityHessianFunction;
    //====================================================
    
    /**
     * @brief Calls the parent validate function, but also checks that none of
     * the function pointers are null (before calling the parent).
     */
    std::optional<std::string> validate() const override;
    
 public:

    FunctionPointerObjective(
            int numVariables,
            int numInequalityConstraints,
            int numEqualityConstraints,
            int numParameters);

    int numParameters() const { return this->_numParameters; }
    void setParameters(Eigen::VectorXd parameters);

 public:

    int numVariables() const override { return this->_numVariables; }
    int numInequalityConstraints() const override { return this->_numInequalityConstraints; }
    int numEqualityConstraints() const override { return this->_numEqualityConstraints; }

    double value(const Eigen::VectorXd& state) const override;
    /**
     * @brief Set the function pointer for the value function.
     *
     * @param functionPtr A function pointer to the value function.
     *
     * The value function should expect state and param to be pointers to
     * arrays of doubles, with the length reported by numVariables() and
     * numPointers() respectively (i.e. the number of variables and number of
     * parameters provided when the objective is constructed).
     *
     * The out pointer will point to a double whose value should be the
     * objective value given the state and parameters when the function
     * ValueFunction exits.
     */
    void setValueFunction(ValueFunction functionPtr);

    const Eigen::VectorXd gradient(const Eigen::VectorXd& state) const override;
    /**
     * @brief Set the function pointer for the gradient function.
     *
     * @param functionPtr A function pointer to the gradient function.
     *
     * The gradient function should expect state and param to be pointers to
     * arrays of doubles, with the length reported by numVariables() and
     * numPointers() respectively (i.e. the number of variables and number of
     * parameters provided when the objective is constructed).
     *
     * The out pointer will point to an array of doubles with the same length
     * as the number of variables as reported by numVariables().
     */
    void setGradientFunction(GradientFunction functionPtr);

    const Eigen::MatrixXd hessian(const Eigen::VectorXd& state) const override;
    /**
     * @brief Set the function pointer for the hessian function.
     *
     * @param functionPtr The function pointer to the hessian function.
     *
     * The hessian function should expect state and param to be pointers to
     * arrays of doubles, with the length reported by numVariables() and
     * numPointers() respectively (i.e. the number of variables and number of
     * parameters provided when the objective is constructed).
     *
     * The out pointer will point to an array of doubles with the same length
     * as the number of variables as reported by numVariables() squared. The
     * matrix is in a column-major format.
     */
    void setHessianFunction(HessianFunction functionPtr);

    std::optional<const Eigen::MatrixXd> equalityConstraintMatrix() const override;
    /**
     * @brief Set the function pointer for the equality matrix function.
     *
     * @param functionPtr The function pointer to the equality matrix function.
     *
     * The out pointer will point to an array of doubles with the same length
     * as the number of variables as reported by numVariables() times
     * the number of equality constraints as reported by
     * numEqualityConstraints(). The matrix is in a column-major format.
     *
     * This EqualityMatrixFunction will not be called if the number of
     * equality constraints is 0.
     */
    void setEqualityMatrixFunction(EqualityMatrixFunction functionPtr);

    std::optional<const Eigen::VectorXd> equalityConstraintVector() const override;
    /**
     * @brief Set the function pointer for the equality vector function.
     *
     * @param functionPtr The function pointer for the equality vector function.
     *
     * The out pointer will point to an array of doubles with the same length
     * as the number of equality constraints as reported by
     * numEqualityConstraints().
     * 
     * This EqualityVectorFunction will not be called if the number of
     * equality constraints is 0.
     */
    void setEqualityVectorFunction(EqualityVectorFunction functionPtr);

    double inequalityConstraintsValue(const Eigen::VectorXd& state) const override;
    /**
     * @brief Set the function pointer for the inequality value function.
     *
     * @param functionPtr The function pointer for the inequality value function.
     *
     * The inequality value function should expect state and param to be
     * pointers to arrays of doubles, with the length reported by
     * numVariables() and numPointers() respectively (i.e. the number of
     * variables and number of parameters provided when the objective is
     * constructed).
     *
     * The out pointer will point to an array of doubles with the same length
     * as the number of variables as reported by numVariables(). The
     * InequalityValueFunction will not be called if the number of inequality
     * constraints is 0.
     */
    void setInequalityValueFunction(InequalityValueFunction functionPtr);

    const Eigen::VectorXd inequalityConstraintsGradient(const Eigen::VectorXd& state) const override;
    void  setInequalityGradientFunction(InequalityGradientFunction functionPtr);

    const Eigen::MatrixXd inequalityConstraintsHessian(const Eigen::VectorXd& state) const override;
    void setInequalityHessianFunction(InequalityHessianFunction funcionPtr);
};

} // namespace FastMPC 

} // namespace cppmpc

#endif // INCLUDE_FASTMPCFUNCTIONPOINTEROBJECTIVE_H_
