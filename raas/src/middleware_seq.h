#ifndef RSWEB_RAAS_MIDDLEW_SEQ
#define RSWEB_RAAS_MIDDLEW_SEQ

#include <vector>
#include <string>
#include <tuple>
#include <functional>

#include "middleware/enabled.h"
#include "middleware.h"

namespace rsweb {
    //FIXME: should be extern
    std::vector<middleware_func_type> middleware = {
        mw_check_a_profile_is_active
    };
}

#endif
