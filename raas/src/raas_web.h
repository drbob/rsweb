#ifndef RSWEB_RAAS_HTTP
#define RSWEB_RAAS_HTTP

#include <event2/http.h>
#include <queue>
#include <string>
#include <list>
#include <iostream>
#include <assert.h>
#include <boost/threadpool.hpp>
#include <boost/bind.hpp>
#include "rsweb_entrypoints.h"

namespace rsweb {
typedef boost::threadpool::pool thread_pool;

void request_router(evhttp_request* req) {
    const struct evhttp_uri* url = evhttp_request_get_evhttp_uri(req);
    const char* url_str = evhttp_uri_get_path(url);

    for(auto ep_iter = entrypoints.begin(); ep_iter != entrypoints.end(); ++ep_iter) {
        if((*ep_iter)(url_str, req)) {
            return;
        }
    }
    // no idea, give up
    ep_http_404(req);
}

void queue_request(struct evhttp_request* r, void* arg) {
    thread_pool* tp = static_cast<thread_pool*>(arg);
    assert(tp); assert(r);
    tp->schedule(boost::bind(request_router, r)); 
}
};

#endif
