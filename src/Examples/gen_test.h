// Copyright 2021 Ian Ruh
#ifndef GEN_TEST_H
#define GEN_TEST_H
#include <Eigen/Dense>

Eigen::Matrix<double, -1, -1> getMat(const Eigen::VectorXd& state,
                                     const Eigen::VectorXd& param) {
    Eigen::Matrix<double, -1, -1> A;
    A.resize(10, 10);
    A(0, 0) = state(0);
    A(1, 1) = param(1);
    return A;
}

#endif  // GEN_TEST_H
