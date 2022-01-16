#include "CodeGenerator.h"

#include <fstream>
#include <string>
#include <tuple>
#include <exception>
#include <sstream>
#include "symengine/matrix.h"
#include "symengine/basic.h"
#include "symengine/symbol.h"

#include "OrderedSet.h"
#include "SymbolicEquality.h"
#include "SymEngineUtilities.h"

namespace cppmpc {

using SymEngine::RCP;
using SymEngine::Basic;
using SymEngine::Symbol;


std::pair<std::string, std::string> CodeGenerator::generateSymbolicEqualityCode(
        const SymbolicEqualityConstraints& symbolicConstraints,
        const OrderedSet& variableOrdering,
        const OrderedSet& parameterOrdering,
        const std::string& matrixFunctionName,
        const std::string& vectorFunctionName) {
    // Get the linearized matrices.
    SymEngine::DenseMatrix mat;
    SymEngine::DenseMatrix vec;
    std::tie(mat, vec) = symbolicConstraints.convertToLinearSystem(variableOrdering);

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
    ssMat << "void " << matrixFunctionName << "(const double* param, double* out) {" << std::endl;

    // The actual matrix construction code
    ssMat << generateCCode(mat, variableRepr, parameterRepr, "out");

    ssMat << "}" << std::endl;

    //============= Equality Vector ===========
    std::stringstream ssVec;

    // Function signature
    ssVec << "void " << vectorFunctionName << "(const double* param, double* out) {" << std::endl;

    // The actual matrix construction code
    ssVec << generateCCode(vec, variableRepr, parameterRepr, "out");

    ssVec << "}" << std::endl;

    return std::make_pair(ssMat.str(), ssVec.str());
}

void CodeGenerator::writeFunctionsToFile(
        const std::string& filePath,
        const std::vector<std::string>& functionStrings) {

    std::stringstream ss;

    ss << "#include \"math.h\"" << std::endl;

    ss << "#ifdef __cplusplus" << std::endl;
    ss << "extern \"C\" {" << std::endl;
    ss << "#endif" << std::endl << std::endl;

    for (std::string str: functionStrings) {
        ss << str << std::endl << std::endl;
    }

    ss << "#ifdef __cplusplus" << std::endl;
    ss << "}" << std::endl;
    ss << "#endif" << std::endl;

    std::ofstream file;
    file.open(filePath);
    file << ss.str();
    file.close(); 
}

} // namespace cppmpc
