#ifndef INCLUDE_CODEGENERATOR_H_
#define INCLUDE_CODEGENERATOR_H_

#include "symengine/matrix.h"

#include "SymbolicEquality.h"
#include "Util.h"
#include "OrderedSet.h"
#include <string>
#include <tuple>
#include <exception>
#include <vector>

namespace cppmpc {

class CodeGenerator {
 public:


    /**
     * @brief Generate C code that constructs an eigen matrix equivalent to the
     * passed symbolic matrix, using the given strings as representations for the
     * variables and parameters.
     *
     * The matrix elements are set in columns major order.
     *
     * @param mat The symbolic mat the matrix eigne matrix is based on.
     * @param variableRepr A map of variables to symbols.
     * @param parameterRepr A map of parameters to symbols.
     * @param matrixName The name of the matrix variable in the generated code.
     */
    static std::string generateDenseMatrixCode(const SymEngine::DenseMatrix& mat,
                          const MapBasicString& variableRepr,
                          const MapBasicString& parameterRepr,
                          const std::string& matrixName);
    
    /**
     * @brief Generate code that can be compiled and used to calculate the
     * equality matrix and vector.
     *
     * @param symbolicConstraints The symbolic equality constraints to generate
     * code for
     * @param variableOrdering Variable ordering to use.
     * @param parameterOrdering Parameter ordering to use.
     * @param functionName The name of the function to generate.
     */
    static std::pair<std::string, std::string> generateSymbolicEqualityFunctions(
            const SymbolicEqualityConstraints& symbolicConstraints,
            const OrderedSet& variableOrdering,
            const OrderedSet& parameterOrdering,
            const std::string& matrixFunctionName,
            const std::string& vectorFunctionName);

    static void writeFunctionsToFile(
            const std::string& filePath,
            const std::vector<std::string>& functionStrings);
};

} // namespace cppmpc

#endif // INCLUDE_CODEGENERATOR_H_
