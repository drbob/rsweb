#include "../middleware.h"
#include "../raas.h"
#include "../entrypoint.h"
#include <boost/algorithm/string/predicate.hpp>

namespace rsweb {
    evhttp_request* mw_check_a_profile_is_active(evhttp_request* req) {
        if(rsweb::rs_control_startup_retroshare_called) return req;
        else {
            // work out if this request maps to an ep that we think is safe
            //FIXME: middleware should also receive a destination ep to be called along
            //with the requestion object but we dont have a suitable request type
            //or deferable ep's yet

            // extract the path from the URI (drop query and fragment) and decode it
            const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
            std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

            if(boost::starts_with(path, "/profile")) return req;
            else if(boost::starts_with(path, "/static")) return req;
            else ep_http_500(req);
            return NULL;
        }
    }

}

