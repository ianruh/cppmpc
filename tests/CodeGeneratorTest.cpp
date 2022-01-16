// Copyright 2021 Ian Ruh
#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <sstream>
#include <dlfcn.h>

#include <symengine/expression.h>

#include "OrderedSet.h"
#include "SymEngineUtilities.h"
#include "SymbolicEquality.h"
#include "CodeGenerator.h"
#include "Util.h"

using namespace SymEngine;
using namespace cppmpc;

typedef void (*EqualityMatrixFunction)(const double* param, double* out);
typedef void (*EqualityVectorFunction)(const double* param, double* out);

TEST(CodeGeneratorTests, SymbolicEquality) {
    // Construct the symbolic equality constraints
    Expression x = Expression(variable("x"));
    Expression y = Expression(variable("y"));
    Expression z = Expression(variable("z"));
    Expression a = Expression(parameter("a"));
    OrderedSet variableOrdering = OrderedSet();
    variableOrdering.append(x);
    variableOrdering.append(y);
    variableOrdering.append(z);
    OrderedSet parameterOrdering = OrderedSet();
    parameterOrdering.append(a);

    SymbolicEqualityConstraints constraints = SymbolicEqualityConstraints();
    constraints.appendConstraint(x, 3 * y + 4);
    constraints.appendConstraint((z + a) / 2, 7);

    std::vector<std::string> functions;

    std::string mat;
    std::string vec;
    std::tie(mat,vec) = CodeGenerator::generateSymbolicEqualityCode(
            constraints,
            variableOrdering,
            parameterOrdering,
            "equalityMatrix",
            "equalityVector");
    functions.push_back(mat);
    functions.push_back(vec);

    std::string tempFileBase = std::tmpnam(nullptr);

    std::string tempFile = tempFileBase + std::string(".cpp");    
    std::cout << "Wrote temp file: " << tempFile << std::endl;
    CodeGenerator::writeFunctionsToFile(
            tempFile,
            functions);

    std::string tempSharedObject = tempFileBase + std::string(".so");

    std::stringstream cmd;
    cmd << CPP_COMPILER_PATH << " -shared " << tempFile << " -o " << tempSharedObject;
    int rt = std::system(cmd.str().c_str());

    // Check the compiling succeeded
    ASSERT_EQ(rt, 0);

    // load library        
    void* sharedLib = dlopen(tempSharedObject.c_str(), RTLD_LAZY);
    if (!sharedLib) {
        std::stringstream msg;
        msg << "Cannot open library: " << dlerror() << std::endl;
        FAIL() << msg.str();
    }

    EqualityMatrixFunction equalityMatrixFunction = (EqualityMatrixFunction)dlsym(
            sharedLib,
            "equalityMatrix");
    EqualityVectorFunction equalityVectorFunction = (EqualityMatrixFunction)dlsym(
            sharedLib,
            "equalityVector");

    if (!equalityMatrixFunction) {
        FAIL() << "equalityMatrix function failed to resolve.";
    }
    if (!equalityVectorFunction) {
        FAIL() << "equalityVector function failed to resolve.";
    }

    Eigen::VectorXd param(1);
    param << 1.0;
    Eigen::VectorXd outputVector(2);
    Eigen::MatrixXd outputMat(2, 3);

    (*equalityMatrixFunction)(param.data(), outputMat.data());
    (*equalityVectorFunction)(param.data(), outputVector.data());

    EXPECT_NEAR(outputMat(0,0), 1.0, 1e-8);
    EXPECT_NEAR(outputMat(0,1), -3, 1e-8);
    EXPECT_NEAR(outputMat(0,2), 0.0, 1e-8);
    EXPECT_NEAR(outputMat(1,0), 0.0, 1e-8);
    EXPECT_NEAR(outputMat(1,1), 0.0, 1e-8);
    EXPECT_NEAR(outputMat(1,2), 0.5, 1e-8);

    EXPECT_NEAR(outputVector(0), 4.0, 1e-8);
    EXPECT_NEAR(outputVector(1), 6.5, 1e-8);

    dlclose (sharedLib);

}
