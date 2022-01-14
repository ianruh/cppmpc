#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_
#include <iostream>

// DEBUG macro
#define DEBUG(x) do { std::cerr <<  __FILE__ << ":" << __LINE__ << " " << #x << ": " << x << std::endl; } while (0)

#endif // INCLUDE_UTIL_H_
