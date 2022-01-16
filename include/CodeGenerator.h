#ifndef INCLUDE_CODEGENERATOR_H_
#define INCLUDE_CODEGENERATOR_H_

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
     * @brief Generate code that can be compiled and used to calculate the
     * equality matrix and vector.
     *
     * @param symbolicConstraints The symbolic equality constraints to generate
     * code for
     * @param variableOrdering Variable ordering to use.
     * @param parameterOrdering Parameter ordering to use.
     * @param functionName The name of the function to generate.
     */
    static std::pair<std::string, std::string> generateSymbolicEqualityCode(
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
