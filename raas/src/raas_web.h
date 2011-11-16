#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <queue>
#include <string>
#include <list>
#include <iostream>
#include <assert.h>
#include <boost/threadpool.hpp>
#include <boost/bind.hpp>
#include <uriparser/Uri.h>
#include <retroshare/rspeers.h>

namespace rsweb {
// the request thread body
// no actual request processing happens until
// we get here. everything else is just bouncing things
// between threads
void request_router(evhttp_request* req) {
    struct evbuffer* resp = evbuffer_new();

    std::list<std::string> ssl_friends;
    rsPeers->getFriendList(ssl_friends);

    for(auto iter = ssl_friends.begin(); iter != ssl_friends.end(); ++iter) {
        evbuffer_add(resp, iter->c_str(), iter->size());
        std::cout << "friend " << *iter << rsPeers->getPeerName(*iter) << ":" << rsPeers->isOnline(*iter) << std::endl;
    }

    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 200, "OK", resp);
}

typedef boost::threadpool::pool thread_pool;

void queue_request(struct evhttp_request* r, void* arg) {
    thread_pool* tp = static_cast<thread_pool*>(arg);
    assert(tp); assert(r);
    tp->schedule(boost::bind(request_router, r)); 
}
};
