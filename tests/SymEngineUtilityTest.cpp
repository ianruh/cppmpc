// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>
#include <symengine/basic.h>
#include <symengine/expression.h>
#include <symengine/matrix.h>
#include <symengine/symbol.h>
#include <symengine/integer.h>
#include <iostream>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"

using SymEngine::Basic;
using SymEngine::Expression;
using SymEngine::RCP;
using SymEngine::Symbol;

TEST(SymEngineUtilityTests, Gradient) {
    RCP<const Symbol> xb = SymEngine::symbol("x");
    RCP<const Symbol> yb = SymEngine::symbol("y");
    RCP<const Symbol> zb = SymEngine::symbol("z");

    auto x = Expression(xb);
    auto y = Expression(yb);
    auto z = Expression(zb);

    cppmpc::OrderedSet set;
    set.append(xb);
    set.append(yb);
    set.append(zb);

    Expression exp = x * x * x + 2 * x * y + z * z * y;

    SymEngine::DenseMatrix grad = cppmpc::gradient(exp.get_basic(), set);

    EXPECT_TRUE((3 * x * x + 2 * y).get_basic()->compare(*grad.get(0, 0)) == 0);
    EXPECT_TRUE((2 * x + z * z).get_basic()->compare(*grad.get(1, 0)) == 0);
    EXPECT_TRUE((2 * z * y).get_basic()->compare(*grad.get(2, 0)) == 0);
}

TEST(SymEngineUtilityTests, Jacobian) {
    RCP<const Symbol> xb = SymEngine::symbol("x");
    RCP<const Symbol> yb = SymEngine::symbol("y");
    RCP<const Symbol> zb = SymEngine::symbol("z");

    auto x = Expression(xb);
    auto y = Expression(yb);
    auto z = Expression(zb);

    cppmpc::OrderedSet set;
    set.append(xb);
    set.append(yb);
    set.append(zb);

    // The function f
    Expression f1 = x*x*x + 2*x*y + z*z*y;
    Expression f2 = 3*y + z;
    Expression f3 = 7*x*y;
    SymEngine::DenseMatrix f(3, 1);
    f.set(0, 0, f1.get_basic());
    f.set(1, 0, f2.get_basic());
    f.set(2, 0, f3.get_basic());

    SymEngine::DenseMatrix jacobian = cppmpc::jacobian(f, set);

    EXPECT_TRUE((3*x*x + 2*y).get_basic()->compare(*jacobian.get(0, 0)) == 0);
    EXPECT_TRUE((2*x + z*z).get_basic()->compare(*jacobian.get(0, 1)) == 0);
    EXPECT_TRUE((2*y*z).get_basic()->compare(*jacobian.get(0, 2)) == 0);
    EXPECT_TRUE(Expression(0).get_basic()->compare(*jacobian.get(1, 0)) == 0);
    EXPECT_TRUE(Expression(3).get_basic()->compare(*jacobian.get(1, 1)) == 0);
    EXPECT_TRUE(Expression(1).get_basic()->compare(*jacobian.get(1, 2)) == 0);
    EXPECT_TRUE((7*y).get_basic()->compare(*jacobian.get(2, 0)) == 0);
    EXPECT_TRUE((7*x).get_basic()->compare(*jacobian.get(2, 1)) == 0);
    EXPECT_TRUE(Expression(0).get_basic()->compare(*jacobian.get(2, 2)) == 0);
}

TEST(SymEngineUtilityTests, Hessian) {
    RCP<const Symbol> xb = SymEngine::symbol("x");
    RCP<const Symbol> yb = SymEngine::symbol("y");
    RCP<const Symbol> zb = SymEngine::symbol("z");

    auto x = Expression(xb);
    auto y = Expression(yb);
    auto z = Expression(zb);

    cppmpc::OrderedSet set;
    set.append(xb);
    set.append(yb);
    set.append(zb);

    Expression exp = x * x * x + 2 * x * y + z * z * y;

    SymEngine::DenseMatrix hess = cppmpc::hessian(exp.get_basic(), set);

    std::cout << hess.__str__() << std::endl;
    // Row 1
    // Derivative wrt x: 3*x*x + 2*y
    EXPECT_TRUE((6 * x).get_basic()->compare(*hess.get(0, 0)) == 0);
    EXPECT_TRUE(Expression(2).get_basic()->compare(*hess.get(0, 1)) == 0);
    EXPECT_TRUE(Expression(0).get_basic()->compare(*hess.get(0, 2)) == 0);

    // Row 2
    // Derivative wrt y: 2*x + z*z
    EXPECT_TRUE(Expression(2).get_basic()->compare(*hess.get(1, 0)) == 0);
    EXPECT_TRUE(Expression(0).get_basic()->compare(*hess.get(1, 1)) == 0);
    EXPECT_TRUE((2 * z).get_basic()->compare(*hess.get(1, 2)) == 0);

    // Row 3
    // Derivative wrt z: 2*z*y
    EXPECT_TRUE(Expression(0).get_basic()->compare(*hess.get(2, 0)) == 0);
    EXPECT_TRUE(Expression(2 * z).get_basic()->compare(*hess.get(2, 1)) == 0);
    EXPECT_TRUE(Expression(2 * y).get_basic()->compare(*hess.get(2, 2)) == 0);
}
