cmake_minimum_required(VERSION 3.14)
project(cppmpc)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 17)

# Enable Compiler warnings
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# SWIG generates some code that this warning isn't happy with
add_compile_options(-Wno-missing-field-initializers)
# Make SWIG happy (something to do with dynamic linking)
add_compile_options(-fPIC)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

################################# Dependencies ################################

######## SymEngine ########
# TODO(ianruh): Make this optional so solvers can be built without needing
# symengine installed.
FIND_PACKAGE(SymEngine REQUIRED PATHS third_party/install)

# I don't know why this is needed. Shouldn't gmp be linked when SymEngine
# builds since it is building static libs?
FIND_LIBRARY(gmp NAMES gmp libgmp)

######## Google Test ########
# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

####### Eigen #######
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

################################# Targets #####################################

######## Libraries ########

# cppmpc Library
add_library(cppmpc
    src/SymbolicObjective.cpp
    src/SymEngineUtilities.cpp
    src/GetSymbolsVisitor.cpp
    src/SymbolicEquality.cpp
    include/SymbolicObjective.h
    include/SymEngineUtilities.h
    include/GetSymbolsVisitor.h
    include/SymbolicEquality.h
    include/OrderedSet.h
)
target_include_directories(cppmpc PUBLIC include/)
target_link_libraries(cppmpc symengine gmp Eigen3::Eigen)

######## Executables ########

add_executable(example1 src/Examples/example1.cpp)
target_link_libraries(example1 PRIVATE cppmpc symengine)

######## Tests ########
enable_testing()

add_executable(SymbolicTests
    tests/SymbolicObjectiveTest.cpp
    tests/GetSymbolsVisitorTest.cpp
    tests/OrderedSetTest.cpp
    tests/EqualityConstraintTest.cpp)
target_link_libraries(SymbolicTests cppmpc gtest_main symengine)

include(GoogleTest)
gtest_discover_tests(SymbolicTests)


