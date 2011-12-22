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

#include "url_map.h"
#include "entrypoint.h"
#include "middleware.h"
#include "middleware_seq.h"
#include "http_errors.h"

namespace rsweb {
typedef boost::threadpool::pool thread_pool;

void request_router(evhttp_request* req) {
    const struct evhttp_uri* url = evhttp_request_get_evhttp_uri(req);
    const char* url_str = evhttp_uri_get_path(url);

    // prior to handing the request over to the entrypoint we 
    // need to pass it through the middleware chain.
    // this is basically just a series of functions with the following type
    // request* middleware(request*)
    // a NULL return means that the request has been short-circuited by
    // the middleware and that we should stop processing it
    for(auto mw_iter = middleware.begin(); mw_iter != middleware.end(); ++mw_iter) {
        if(!(req = (*mw_iter)(req))) {
            return;
        }
    }

    // now despatch the request to the entrypoint that deals with the page
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
    // delay the actual routing of the request until a thread is available
    tp->schedule(boost::bind(request_router, r)); 
}

}

#endif
