// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>
#include <symengine/basic.h>
#include <symengine/symbol.h>

#include "OrderedSet.h"

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

TEST(OrderedSetTests, Basics) {
    RCP<const Symbol> x = SymEngine::symbol("x");
    RCP<const Symbol> y = SymEngine::symbol("y");
    RCP<const Symbol> z = SymEngine::symbol("z");

    cppmpc::OrderedSet set;

    EXPECT_TRUE(set.isConsistent());
    set.append(x);
    EXPECT_TRUE(set.isConsistent());
    set.append(y);
    EXPECT_TRUE(set.isConsistent());
    set.append(z);

    EXPECT_EQ(set.at(0), x);
    EXPECT_EQ(set.at(1), y);
    EXPECT_EQ(set.at(2), z);

    EXPECT_EQ(3, set.size());

    set.remove(0);
    EXPECT_TRUE(set.isConsistent());
    EXPECT_EQ(set.at(0), y);
    EXPECT_EQ(set.at(1), z);

    EXPECT_EQ(2, set.size());
}
