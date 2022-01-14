#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_
#include <iostream>
#include <Eigen/Dense>

Eigen::IOFormat FlatFmt(4, 0, ", ", ";", "", "", "[", "]");

#ifdef DEBUG
#define DEBUG_PRINT(x) { \
    do { std::cerr <<  __FILE__ << ":" << __LINE__ << " " << x << std::endl; } while (0); \
}
#else
#define DEBUG_PRINT(X)
#endif // DEBUG

#endif // INCLUDE_UTIL_H_
