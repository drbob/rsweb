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
#include <uriparser/Uri.h>
#include "rsweb_entrypoints.h"

namespace rsweb {
typedef boost::threadpool::pool thread_pool;

void ep_http_404(evhttp_request* req) {
    struct evbuffer* resp = evbuffer_new();
    evbuffer_add_printf(resp, "404 :(\n%s", evhttp_request_get_uri(req)); 

    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 404, "Not Found", resp);
}

void request_router(evhttp_request* req) {
    const char* url_str = evhttp_request_get_uri(req);
    
    for(auto ep_iter = entrypoints.begin(); ep_iter != entrypoints.end(); ++ep_iter) {
        if(std::get<0>(*ep_iter) == url_str) {
            std::get<1>(*ep_iter)(req);
            return;
        }
    }

    // no idea, give up
    ep_http_404(req);
}

    
void queue_request(struct evhttp_request* r, void* arg) {
    thread_pool* tp = static_cast<thread_pool*>(arg);
    assert(tp); assert(r);
    evhttp_request_own(r);
    tp->schedule(boost::bind(request_router, r)); 
}
};

#endif
