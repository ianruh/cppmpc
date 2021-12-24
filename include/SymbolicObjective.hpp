#ifndef _SYMBOLIC_OBJECTIVE_H
#define _SYMBOLIC_OBJECTIVE_H

#include <symengine/basic.h>

using SymEngine::RCP;
using SymEngine::Basic;

namespace cppmpc {
    
    class SymbolicObjective {
        
        // Constructors
        public:
            SymbolicObjective();

            void print(const RCP<const Basic> &basic);

    };

} // namespace cppmpc

#endif // _SYMBOLIC_OBJECTIVE_H
