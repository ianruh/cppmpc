#ifndef _SYMENGINE_UTILITIES_H
#define _SYMENGINE_UTILITIES_H

#include <symengine/basic.h>

namespace cppmpc {

    using SymEngine::RCP;
    using SymEngine::Basic;

    namespace Utility {

        // This is a utility just used to test that the swig wrappers can convert to
        // and from RCP<const Basic>.
        const RCP<const Basic>& echo(const RCP<const Basic> &basic);

    } // namespace Utility

} // namespace cppmpc

#endif // _SYMBOLIC_OBJECTIVE_H
