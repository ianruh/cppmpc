cmake_minimum_required(VERSION 3.14)
project(cppmpc)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

# Enable Compiler warnings
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options()
endif()

# SWIG generates some code that this warning isn't happy with
#add_compile_options(-Wno-missing-field-initializers)
# Make SWIG happy (something to do with dynamic linking)
#add_compile_options(-fPIC)
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-03")

# Set debug flags. These don't enable the debug flags, they just tell cmake
# which flags to set in debug mode.
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

################################# Dependencies ################################
include(FetchContent)

######## SymEngine ########
# TODO(ianruh): Make this optional so solvers can be built without needing
# symengine installed.
FetchContent_Declare(
    symengine
    GIT_REPOSITORY https://github.com/symengine/symengine.git
    GIT_TAG        v0.9.0
)
FetchContent_MakeAvailable(symengine)
include_directories(SYSTEM ${symengine_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/symengine-build/)
set(BUILD_TESTS OFF CACHE INTERNAL "SymEngine Test Building")
set(BUILD_BENCHMARKS OFF CACHE INTERNAL "SymEngine Benchmark Building")

# I don't know why this is needed. Shouldn't gmp be linked when SymEngine
# builds since it is building static libs?
FIND_LIBRARY(gmp NAMES gmp libgmp)

######## Google Test ########
# Fetch GoogleTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

####### Eigen #######
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

####### pybind11 #######
#FetchContent_Declare(
#    pybind11
#    GIT_REPOSITORY https://github.com/pybind/pybind11
#    GIT_TAG        v2.2.3
#)
#FetchContent_MakeAvailable(pybind11)
#
#FetchContent_GetProperties(pybind11)
#if(NOT pybind11_POPULATED)
#    FetchContent_Populate(pybind11)
#    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
#endif()
#target_compile_options(pybind11 INTERFACE -Wno-deprecated-declarations)

################################# Targets #####################################

######## Libraries ########

# cppmpc Library
add_library(cppmpc
    cppmpc/SymbolicObjective.cpp
    cppmpc/SymEngineUtilities.cpp
    cppmpc/GetSymbolsVisitor.cpp
    cppmpc/SymbolicEquality.cpp
    cppmpc/FastMPC.cpp
    cppmpc/FastMPCFunctionPointerObjective.cpp
    cppmpc/CodeGenerator.cpp
    cppmpc/SymbolicInequality.cpp
    cppmpc/SymbolicObjective.h
    cppmpc/SymEngineUtilities.h
    cppmpc/GetSymbolsVisitor.h
    cppmpc/SymbolicEquality.h
    cppmpc/OrderedSet.h
    cppmpc/FastMPC.h
    cppmpc/FastMPCFunctionPointerObjective.h
    cppmpc/CodeGenerator.h
    cppmpc/SymbolicInequality.h
)
target_include_directories(cppmpc PUBLIC cppmpc/)
target_link_libraries(cppmpc symengine gmp Eigen3::Eigen ${CMAKE_DL_LIBS})
target_compile_options(cppmpc PUBLIC -Wall -Wextra -Wpedantic -Werror)

######## Executables ########

#add_executable(CartPole cppmpc/Examples/CartPole.cpp)
#target_link_libraries(CartPole PRIVATE cppmpc symengine pybind11::embed)
#target_compile_options(CartPole PUBLIC -Wall -Wextra -Wpedantic -Werror)

######## Tests ########
enable_testing()

add_executable(SymbolicTests
    tests/SymbolicObjectiveTest.cpp
    tests/GetSymbolsVisitorTest.cpp
    tests/OrderedSetTest.cpp
    tests/EqualityConstraintTest.cpp
    tests/FastMPCSimpleObjectiveTest.cpp
    tests/FastMPCFunctionPointerObjectiveTest.cpp
    tests/CodeGeneratorTest.cpp
    tests/SymEngineUtilityTest.cpp)
target_link_libraries(SymbolicTests cppmpc gtest_main symengine Eigen3::Eigen)
target_compile_options(SymbolicTests PUBLIC -Wall -Wextra -Wpedantic -Werror)

include(GoogleTest)
gtest_discover_tests(SymbolicTests)
