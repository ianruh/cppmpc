// Copyright 2021 Ian Ruh
#include <iostream>

#include "SymbolicObjective.h"
#include "symengine/basic.h"
#include "symengine/printers.h"
#include "symengine/symbol.h"

#include <Eigen/Dense>
#include "gen_test.h"

int main(int, char**) {
    // cppmpc::SymbolicObjective obj;

    Eigen::VectorXd a(3);
    a << 1.0, 2.0, 3.0;
    Eigen::VectorXd b(3);
    b << 4.0, 5.0, 6.0;

    Eigen::Matrix<double, -1, -1> mat = getMat(a, b);

    std::cout << mat << std::endl;

    return 0;
}
