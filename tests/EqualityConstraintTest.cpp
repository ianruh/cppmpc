// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>

#include <symengine/basic.h>
#include <symengine/symbol.h>
#include <symengine/add.h>
#include <symengine/mul.h>
#include <symengine/printers.h>
#include <symengine/expression.h>
#include <symengine/integer.h>
#include <symengine/rational.h>

#include "SymEngineUtilities.h"
#include "OrderedSet.h"
#include "SymbolicEquality.h"

// Don't do this in real code, but since polluting here doesn't really matter,
// I'm too lazy.
using namespace SymEngine;
using namespace cppmpc;

TEST(SymbolicEqualityTests, BasicLinearization) {
    // Variables
    Expression x = Expression(variable("x"));
    Expression y = Expression(variable("y"));
    Expression z = Expression(variable("z"));
    
    // Parameter
    Expression a = Expression(parameter("a"));
    
    // Reverse ordering to make life more interesting
    OrderedSet ordering = OrderedSet();
    ordering.append(z);
    ordering.append(y);
    ordering.append(x);
    
    // Add the constraints
    SymbolicEqualityConstraints constraints = SymbolicEqualityConstraints();
    constraints.appendConstraint(x, 3*y + 4);
    constraints.appendConstraint((z+a)/2, 7);

    // Pull out the matrices
    DenseMatrix mat;
    DenseMatrix vector;
    std::tie(mat, vector) = constraints.convertToLinearSystem(ordering);

    // Expands each element in the matrices
    expandAll(mat);
    expandAll(vector);

    // What  we expect
    DenseMatrix expectedMat = DenseMatrix(2, 3, {
            integer(0), integer(-3), integer(1),
            rational(1,2), integer(0), integer(0)
    });

    DenseMatrix expectedVector = DenseMatrix(2,1, {
        integer(4),
        sub(integer(7), div(a.get_basic(),integer(2)))
    });
    
    // Compilation fails for some reason when usinf EXPECT_EQ, but C++'s error
    // messages are so bad I can't tell why.
    EXPECT_TRUE(mat == expectedMat);
    EXPECT_TRUE(vector == expectedVector);
    
}
