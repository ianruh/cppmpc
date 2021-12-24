#include <SymEngineUtilities.hpp>
#include <symengine/basic.h>

namespace cppmpc {

    namespace Utility {

        const RCP<const Basic>& echo(const RCP<const Basic> &basic) {
            return basic;
        }

    } // namespace Utility

} // namespace cppmpc
