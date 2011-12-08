#ifndef RSWEB_MIDDLEW_ENABLED_H
#define RSWEB_MIDDLEW_ENABLED_H
#include <event2/http.h>

//FIXME: should probably actually just include a bunch of .h files instead of having the declarations in here

namespace rsweb {
    evhttp_request* mw_check_a_profile_is_active(evhttp_request*);
}

#endif
