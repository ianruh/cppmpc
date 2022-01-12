// Copyright 2021 Ian Ruh
#include "SymbolicEquality.h"

#include <symengine/basic.h>
#include <symengine/expression.h>
#include <symengine/matrix.h>
#include <symengine/polys/basic_conversions.h>
#include <symengine/sets.h>
#include <symengine/subs.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

void util_union(UnorderedSetSymbol& base, const UnorderedSetSymbol& other) {
    for (auto el : other) {
        base.insert(el);
    }
}

void SymbolicEqualityConstraints::appendConstraint(const RCP<const Basic>& b) {
    this->insertConstraint(this->numConstraints(), b);
}

void SymbolicEqualityConstraints::appendConstraint(const Expression& left,
                                                   const Expression& right) {
    this->insertConstraint(this->numConstraints(), left, right);
}

void SymbolicEqualityConstraints::removeConstraint(size_t index) {
    this->constraints.erase(this->constraints.begin() + index);
}

const RCP<const Basic>& SymbolicEqualityConstraints::getConstraint(
        size_t index) const {
    return this->constraints.at(index);
}

void SymbolicEqualityConstraints::insertConstraint(size_t index,
                                                   const RCP<const Basic>& b) {
    this->constraints.insert(this->constraints.begin() + index, b);
}

void SymbolicEqualityConstraints::insertConstraint(size_t index,
                                                   const Expression& left,
                                                   const Expression& right) {
    this->insertConstraint(index,
                           SymEngine::sub(left.get_basic(), right.get_basic()));
}

UnorderedSetSymbol SymbolicEqualityConstraints::getSymbols() const {
    UnorderedSetSymbol allSymbols;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(allSymbols, cppmpc::getSymbols(b));
    }
    return allSymbols;
}

UnorderedSetSymbol SymbolicEqualityConstraints::getVariables() const {
    UnorderedSetSymbol variables;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(variables, cppmpc::getVariables(b));
    }
    return variables;
}

UnorderedSetSymbol SymbolicEqualityConstraints::getParameters() const {
    UnorderedSetSymbol parameters;
    for (const RCP<const Basic>& b : this->constraints) {
        util_union(parameters, cppmpc::getParameters(b));
    }
    return parameters;
}

SymEngine::set_basic getSymEngineSet(const OrderedSet& variables) {
    SymEngine::set_basic sb;
    for (size_t i = 0; i < variables.size(); i++) {
        sb.insert(variables.at(i));
    }
    return sb;
}

std::pair<SymEngine::DenseMatrix, SymEngine::DenseMatrix>
SymbolicEqualityConstraints::convertToLinearSystem(
        const OrderedSet& variableOrdering) const {
    UnorderedSetSymbol unorderedVariables = this->getVariables();

    // Verify that the variableOrdering is a superset of the variables in the
    // constraints.
    for (auto it = unorderedVariables.cbegin(); it != unorderedVariables.end();
         ++it) {
        if (!variableOrdering.contains(*it)) {
            throw std::runtime_error(
                    "Variable ordering is not a super set of the variables in "
                    "the equality constraints.");
        }
    }

    // Create the constraints matrix and vector
    size_t numConstraints = this->numConstraints();
    size_t numVariables = variableOrdering.size();

    SymEngine::DenseMatrix constraintsMatrix(numConstraints, numVariables);
    zeros(constraintsMatrix);
    SymEngine::DenseMatrix constantsVector(numConstraints, 1);
    zeros(constantsVector);

    SymEngine::set_basic gens = getSymEngineSet(variableOrdering);
    SymEngine::umap_basic_uint index_of_sym;
    for (size_t i = 0; i < variableOrdering.size(); i++) {
        index_of_sym[variableOrdering.at(i)] = i;
    }

    // Loop through each constraint (e.g. each row of the matrix)
    size_t row = 0;
    for (const RCP<const Basic>& equality : this->constraints) {
        // If the top level is an equality constraint, then turn it into an
        // ewuation equal to 0.
        RCP<const Basic> constraint = equality;
        if (SymEngine::is_a<SymEngine::Equality>(*equality)) {
            constraint = SymEngine::sub(
                    SymEngine::down_cast<const SymEngine::Equality&>(*equality)
                            .get_arg2(),
                    SymEngine::down_cast<const SymEngine::Equality&>(*equality)
                            .get_arg1());
        }

        // UnivariateSeries 1 + 2*x + x**2 + O(x**5) has dict_ = {{0, 1}, {1,
        // 2}, {2, 1}} with var_ = "x" and prec_ = 5

        // Construct a polynomial in powers of the variables.
        auto mpoly =
                SymEngine::from_basic<SymEngine::MExprPoly>(constraint, gens);

        // The remainder after going through the variables. This is the term
        // on the constant side of the constraint.
        RCP<const Basic> rem = SymEngine::zero;

        // Loop through each term in the polynomial of the current constraint
        for (const auto& p : mpoly->get_poly().dict_) {
            // Get the term multiplier as a basic
            RCP<const Basic> res = (p.second.get_basic());

            // The index of the current variable being looked at
            size_t whichvar = 0;

            // The count of the number of non-zero terms
            size_t non_zero = 0;

            // Loop through each variable. If this term is linear, then only
            // one variable should be non-zero
            RCP<const Basic> cursim;
            for (auto& sym : gens) {
                // If term is not a constant (e.g. x^0)
                if (0 != p.first[whichvar]) {
                    non_zero++;
                    cursim = sym;
                    // If the power of the current variable is not equal to
                    // one, or if this would be the second non-zero term, then
                    // throw an exception because this constraint can't be
                    // linear.
                    if (p.first[whichvar] != 1 || non_zero == 2) {
                        throw std::runtime_error("Expected a linear equation.");
                    }
                }
                whichvar++;
            }
            if (non_zero == 0) {
                // If all variable powers were zero, then the entire term is
                // constant.
                rem = res;
            } else {
                // Set the given element in the matrix to the current term.
                constraintsMatrix.set(row, index_of_sym[cursim], res);
            }
        }
        constantsVector.set(row, 0, neg(rem));
        row += 1;
    }
    return std::make_pair(constraintsMatrix, constantsVector);
}

std::string SymbolicEqualityConstraints::generateMatrixFunctionString(
        const OrderedSet& variableOrdering, const OrderedSet& parameterOrdering,
        const std::string& functionName) const {
    // Get the linearized matrices.
    SymEngine::DenseMatrix mat;
    SymEngine::DenseMatrix vec;
    std::tie(mat, vec) = this->convertToLinearSystem(variableOrdering);

    // Get all parameters and variables
    UnorderedSetSymbol parameters = this->getParameters();
    UnorderedSetSymbol variables = this->getVariables();

    // Verify that all parameters have a representation
    for (RCP<const Symbol> parameter : parameters) {
        if (!parameterOrdering.contains(parameter)) {
            throw std::runtime_error(
                    "Not all paramaters have a representations");
        }
    }

    // Create the representations for the parameters
    MapBasicString parameterRepr;
    for (RCP<const Symbol> parameter : parameters) {
        parameterRepr[parameter] =
                "param[" +
                std::to_string(parameterOrdering.indexOf(parameter)) + "]";
    }

    // Create the representations for the variables.
    MapBasicString variableRepr;
    for (RCP<const Symbol> variable : variables) {
        variableRepr[variable] =
                "state[" + std::to_string(variableOrdering.indexOf(variable)) +
                "]";
    }

    std::string matrixName = "A";

    std::stringstream ss;

    // Function signature
    ss << "Eigen::MatrixXd " << functionName;
    ss << "(const Eigen::VectorXd& state, const Eigen::VectorXd& param) {";
    ss << std::endl;

    // The actual matrix xonstruction code
    ss << generateCCode(mat, variableRepr, parameterRepr, matrixName);

    ss << "return " << matrixName << ";" << std::endl;
    ss << "}" << std::endl;

    return ss.str();
}

}  // namespace cppmpc
