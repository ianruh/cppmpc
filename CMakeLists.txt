cmake_minimum_required(VERSION 3.14)
project(cppmpc)

# GoogleTest requires at least C++11
# Apparenlt unordered map didn't have contains until 20, so we need 20.
set(CMAKE_CXX_STANDARD 20)

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

######## SWIG ########
FIND_PACKAGE(SWIG REQUIRED)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(PythonLibs)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

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

######## SWIG ########
SET(CMAKE_SWIG_FLAGS "-v")
SET(CMAKE_SWIG_OUTDIR ../cppmpc_py)

SET_SOURCE_FILES_PROPERTIES(swig/cppmpc.i PROPERTIES
    CPLUSPLUS ON
)
SWIG_ADD_LIBRARY(cppmpc_swig
    LANGUAGE python
    SOURCES swig/cppmpc.i
)
set_property(TARGET cppmpc_swig PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)
swig_link_libraries(cppmpc_swig PRIVATE cppmpc symengine)
target_compile_options(cppmpc_swig PRIVATE "-Wno-unused-variable" "-Wno-maybe-uninitialized")

# Because this is being loaded by python, you need to set LD_PRELOAD as well:
# LD_PRELOAD=$(gcc -print-file-name=libasan.so)
#target_compile_options(cppmpc_swig PRIVATE "-fsanitize=address")
#target_link_options(cppmpc_swig PRIVATE "-fsanitize=address")

# For some reason setting CMAKE_SWIG_OUTDIR doesn't effect where the shared
# library gets written, so we change it manually here. The _cppmpc_swig target
# is what is created by swig when we add the library.
set_property(TARGET cppmpc_swig PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SWIG_OUTDIR})

######## Executables ########

add_executable(example1 src/Examples/example1.cpp)
target_link_libraries(example1 PRIVATE cppmpc symengine)

######## Tests ########
enable_testing()

add_executable(SymbolicTests
    tests/SymbolicObjectiveTest.cpp
    tests/GetSymbolsVisitorTest.cpp
    tests/OrderedSetTest.cpp)
target_link_libraries(SymbolicTests cppmpc gtest_main symengine)

include(GoogleTest)
gtest_discover_tests(SymbolicTests)


