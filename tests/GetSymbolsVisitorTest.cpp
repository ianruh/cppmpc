// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>
#include <symengine/add.h>
#include <symengine/basic.h>
#include <symengine/mul.h>
#include <symengine/symbol.h>

#include <unordered_set>

#include "GetSymbolsVisitor.h"

TEST(GetSymbolsVisitorTests, BasicTests) {
    SymEngine::RCP<const SymEngine::Symbol> x = SymEngine::symbol("x");

    EXPECT_EQ(cppmpc::getSymbols(x).size(), 1)
            << "The number of symbols in a symbol was not 1";
    EXPECT_EQ(cppmpc::getSymbols(x).count(x), 1)
            << "The symbol in the set returned by getSymbols of a symbol was "
               "the wrong symbol";
}

TEST(GetSymbolsVisitorTests, NestedTests) {
    SymEngine::RCP<const SymEngine::Symbol> x = SymEngine::symbol("x");
    SymEngine::RCP<const SymEngine::Symbol> y = SymEngine::symbol("y");
    SymEngine::RCP<const SymEngine::Symbol> z = SymEngine::symbol("z");

    SymEngine::RCP<const SymEngine::Basic> expr1 = SymEngine::mul(x, y);
    SymEngine::RCP<const SymEngine::Basic> expr2 = SymEngine::add(expr1, z);

    EXPECT_EQ(cppmpc::getSymbols(expr1).size(), 2)
            << "The wrong number of symbols were found";

    auto set = cppmpc::getSymbols(expr2);

    EXPECT_EQ(set.count(x), 1) << "The unordered set is missing a symbol";
    EXPECT_EQ(set.count(y), 1) << "The unordered set is missing a symbol";
    EXPECT_EQ(set.count(z), 1) << "The unordered set is missing a symbol";
}
