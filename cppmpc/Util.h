// Copyright 2021 Ian Ruh
#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_
#include <Eigen/Dense>
#include <iostream>

inline Eigen::IOFormat FlatFmt(4, 0, ", ", ";", "", "", "[", "]");

#ifdef DEBUG
#define DEBUG_PRINT(x)                                                         \
    {                                                                          \
        do {                                                                   \
            std::cerr << __FILE__ << ":" << __LINE__ << " " << x << std::endl; \
        } while (0);                                                           \
    }
#else
#define DEBUG_PRINT(X)
#endif  // DEBUG

#define CPP_COMPILER_PATH "clang"
#define RUNTIME_COMPILER_FLAGS ""

#endif  // INCLUDE_UTIL_H_
