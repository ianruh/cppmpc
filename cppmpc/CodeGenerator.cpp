// Copyright 2021 Ian Ruh
#include "CodeGenerator.h"

#include <symengine/subs.h>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <tuple>
#include "symengine/basic.h"
#include "symengine/matrix.h"
#include "symengine/symbol.h"

#include "OrderedSet.h"
#include "SymEngineUtilities.h"
#include "SymbolicEquality.h"

namespace cppmpc {

using SymEngine::Basic;
using SymEngine::RCP;
using SymEngine::Symbol;

std::string CodeGenerator::generateDenseMatrixCode(
        const SymEngine::DenseMatrix& mat, const MapBasicString& variableRepr,
        const MapBasicString& parameterRepr, const std::string& matrixName) {
    // Verify that each variable  in the matrix has a representation
    // Verify that each parameter in the matrix has a representation
    for (size_t i = 0; i < mat.nrows(); i++) {
        for (size_t j = 0; j < mat.ncols(); j++) {
            UnorderedSetSymbol variables = getVariables(mat.get(i, j));
            UnorderedSetSymbol parameters = getParameters(mat.get(i, j));

            for (auto variable : variables) {
                if (variableRepr.count(variable) != 1) {
                    throw std::runtime_error(
                            "A representation for a variable was not found");
                }
            }
            for (auto parameter : parameters) {
                if (parameterRepr.count(parameter) != 1) {
                    throw std::runtime_error(
                            "A representation for a parameter was not found");
                }
            }
        }
    }

    // Now that we have checked, we can throw them all together
    MapBasicString symbolsMap;
    symbolsMap.insert(variableRepr.begin(), variableRepr.end());
    symbolsMap.insert(parameterRepr.begin(), parameterRepr.end());

    // Construct the map of symbols to replace
    SymEngine::map_basic_basic symbolsRepMap;
    MapBasicString::iterator it;
    for (it = symbolsMap.begin(); it != symbolsMap.end(); it++) {
        symbolsRepMap[it->first] = SymEngine::symbol(it->second);
    }

    std::stringstream ss;

    int count = 0;
    for (size_t col = 0; col < mat.ncols(); col++) {
        for (size_t row = 0; row < mat.nrows(); row++) {
            RCP<const Basic> replaced = SymEngine::expand(
                    SymEngine::xreplace(mat.get(row, col), symbolsRepMap));
            ss << matrixName << "[" << std::to_string(count)
               << "] = " << SymEngine::ccode(*replaced) << ";" << std::endl;
            count += 1;
        }
    }

    return ss.str();
}

std::tuple<std::string, std::string, std::string>
CodeGenerator::generateObjectiveFunctions(
        const RCP<const Basic> symbolicObjective,
        const SymEngine::DenseMatrix& gradientMat,
        const SymEngine::DenseMatrix& hessianMat,
        const OrderedSet& variableOrdering, const OrderedSet& parameterOrdering,
        const std::string& valueFunctionName,
        const std::string& gradientFunctionName,
        const std::string& hessianFunctionName) {
    // Get all parameters and variables
    UnorderedSetSymbol parameters = cppmpc::getParameters(symbolicObjective);
    UnorderedSetSymbol variables = cppmpc::getVariables(symbolicObjective);

    // Create the representations for the parameters
    MapBasicString parameterRepr;
    for (RCP<const Symbol> parameter : parameters) {
        parameterRepr[parameter] =
                "param[" +
                std::to_string(parameterOrdering.indexOf(parameter)) + "]";
    }

    // Create the representations for the variables.
    MapBasicString variableRepr;
    for (RCP<const Symbol> variable : variables) {
        variableRepr[variable] =
                "state[" + std::to_string(variableOrdering.indexOf(variable)) +
                "]";
    }

    //============= Value ===========
    std::stringstream ssValue;

    // Function signature
    ssValue << "void " << valueFunctionName
            << "(const double* state, const double* param, double* out) {"
            << std::endl;

    // The actual matrix construction code
    SymEngine::DenseMatrix valueMat(1, 1);
    valueMat.set(0, 0, symbolicObjective);
    ssValue << CodeGenerator::generateDenseMatrixCode(valueMat, variableRepr,
                                                      parameterRepr, "out");

    ssValue << "}" << std::endl;

    //============= Gradient ===========
    std::stringstream ssGrad;

    // Function signature
    ssGrad << "void " << gradientFunctionName
           << "(const double* state, const double* param, double* out) {"
           << std::endl;

    // The actual matrix construction code
    ssGrad << CodeGenerator::generateDenseMatrixCode(gradientMat, variableRepr,
                                                     parameterRepr, "out");

    ssGrad << "}" << std::endl;

    //============= Hessian ===========
    std::stringstream ssHess;

    // Function signature
    ssHess << "void " << hessianFunctionName
           << "(const double* state, const double* param, double* out) {"
           << std::endl;

    // The actual matrix construction code
    ssHess << CodeGenerator::generateDenseMatrixCode(hessianMat, variableRepr,
                                                     parameterRepr, "out");

    ssHess << "}" << std::endl;

    return std::make_tuple(ssValue.str(), ssGrad.str(), ssHess.str());
}

std::pair<std::string, std::string>
CodeGenerator::generateSymbolicEqualityFunctions(
        const SymbolicEqualityConstraints& symbolicConstraints,
        const OrderedSet& variableOrdering, const OrderedSet& parameterOrdering,
        const std::string& matrixFunctionName,
        const std::string& vectorFunctionName) {
    // Get the linearized matrices.
    SymEngine::DenseMatrix mat;
    SymEngine::DenseMatrix vec;
    std::tie(mat, vec) =
            symbolicConstraints.convertToLinearSystem(variableOrdering);

    // Get all parameters and variables
    UnorderedSetSymbol parameters = symbolicConstraints.getParameters();
    UnorderedSetSymbol variables = symbolicConstraints.getVariables();

    // Verify that all parameteionrs have a representation
    for (RCP<const Symbol> parameter : parameters) {
        if (!parameterOrdering.contains(parameter)) {
            throw std::runtime_error(
                    "Not all paramaters have a representations");
        }
    }

    // Create the representations for the parameters
    MapBasicString parameterRepr;
    for (RCP<const Symbol> parameter : parameters) {
        parameterRepr[parameter] =
                "param[" +
                std::to_string(parameterOrdering.indexOf(parameter)) + "]";
    }

    // Create the representations for the variables.
    MapBasicString variableRepr;
    for (RCP<const Symbol> variable : variables) {
        variableRepr[variable] =
                "state[" + std::to_string(variableOrdering.indexOf(variable)) +
                "]";
    }

    //============= Equality Matrix ===========
    std::stringstream ssMat;

    // Function signature
    ssMat << "void " << matrixFunctionName
          << "(const double* param, double* out) {" << std::endl;

    // The actual matrix construction code
    ssMat << CodeGenerator::generateDenseMatrixCode(mat, variableRepr,
                                                    parameterRepr, "out");

    ssMat << "}" << std::endl;

    //============= Equality Vector ===========
    std::stringstream ssVec;

    // Function signature
    ssVec << "void " << vectorFunctionName
          << "(const double* param, double* out) {" << std::endl;

    // The actual matrix construction code
    ssVec << CodeGenerator::generateDenseMatrixCode(vec, variableRepr,
                                                    parameterRepr, "out");

    ssVec << "}" << std::endl;

    return std::make_pair(ssMat.str(), ssVec.str());
}

std::tuple<std::string, std::string, std::string>
CodeGenerator::generateSymbolicInequalityFunctions(
        const SymbolicInequalityConstraints& symbolicConstraints,
        const OrderedSet& variableOrdering, const OrderedSet& parameterOrdering,
        const std::string& valueFunctionName,
        const std::string& gradientFunctionName,
        const std::string& hessianFunctionName) {
    // Get all parameters and variables
    UnorderedSetSymbol parameters = symbolicConstraints.getParameters();
    UnorderedSetSymbol variables = symbolicConstraints.getVariables();

    // Verify that all parameteionrs have a representation
    for (RCP<const Symbol> parameter : parameters) {
        if (!parameterOrdering.contains(parameter)) {
            throw std::runtime_error(
                    "Not all paramaters have a representations");
        }
    }

    // Create the representations for the parameters
    MapBasicString parameterRepr;
    for (RCP<const Symbol> parameter : parameters) {
        parameterRepr[parameter] =
                "param[" +
                std::to_string(parameterOrdering.indexOf(parameter)) + "]";
    }

    // Create the representations for the variables.
    MapBasicString variableRepr;
    for (RCP<const Symbol> variable : variables) {
        variableRepr[variable] =
                "state[" + std::to_string(variableOrdering.indexOf(variable)) +
                "]";
    }

    // Get the needed matrices
    RCP<const Basic> barrierValue = symbolicConstraints.symbolicBarrierValue();
    SymEngine::DenseMatrix barrierValueMat(1, 1);
    barrierValueMat.set(0, 0, barrierValue);

    SymEngine::DenseMatrix barrierGradientMat =
            symbolicConstraints.symbolicBarrierGradient(variableOrdering);

    SymEngine::DenseMatrix barrierHessianMat =
            symbolicConstraints.symbolicBarrierHessian(variableOrdering);

    //============= Value Function ===========
    std::stringstream ssValue;

    // Function signature
    ssValue << "void " << valueFunctionName
            << "(const double* state, const double* param, double* out) {"
            << std::endl;

    // The actual matrix construction code
    ssValue << CodeGenerator::generateDenseMatrixCode(
            barrierValueMat, variableRepr, parameterRepr, "out");

    ssValue << "}" << std::endl;

    //============= Gradient Function ===========
    std::stringstream ssGrad;

    // Function signature
    ssGrad << "void " << gradientFunctionName
           << "(const double* state, const double* param, double* out) {"
           << std::endl;

    // The actual matrix construction code
    ssGrad << CodeGenerator::generateDenseMatrixCode(
            barrierGradientMat, variableRepr, parameterRepr, "out");

    ssGrad << "}" << std::endl;

    //============= Hessian Function ===========
    std::stringstream ssHess;

    // Function signature
    ssHess << "void " << hessianFunctionName
           << "(const double* state, const double* param, double* out) {"
           << std::endl;

    // The actual matrix construction code
    ssHess << CodeGenerator::generateDenseMatrixCode(
            barrierHessianMat, variableRepr, parameterRepr, "out");

    ssHess << "}" << std::endl;

    return std::make_tuple(ssValue.str(), ssGrad.str(), ssHess.str());
}

void CodeGenerator::writeFunctionsToFile(
        const std::string& filePath,
        const std::vector<std::string>& functionStrings) {
    std::stringstream ss;

    ss << "#include \"math.h\"" << std::endl;

    ss << "#ifdef __cplusplus" << std::endl;
    ss << "extern \"C\" {" << std::endl;
    ss << "#endif" << std::endl << std::endl;

    for (std::string str : functionStrings) {
        ss << str << std::endl << std::endl;
    }

    ss << "#ifdef __cplusplus" << std::endl;
    ss << "}" << std::endl;
    ss << "#endif" << std::endl;

#ifdef DEBUG
    DEBUG_PRINT("Writing temp file: " << filePath);

    for (std::string line; std::getline(ss, line);) {
        DEBUG_PRINT(line);
    }
#endif

    std::ofstream file;
    file.open(filePath);
    file << ss.str();
    file.close();
}

}  // namespace cppmpc
