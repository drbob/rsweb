#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <queue>
#include <string>
#include <list>
#include <iostream>
#include <assert.h>

#include <uriparser/Uri.h>
#include <retroshare/rspeers.h>

namespace rsweb {

class request_queue {
    public:
    pthread_mutex_t qmutex;
    pthread_cond_t empty_condition;

    std::queue<evhttp_request*> queue;
    typedef evhttp_request* value_type;

    request_queue() 
    {
        pthread_mutex_init(&qmutex, NULL);
        pthread_cond_init(&empty_condition, NULL);
    }

    ~request_queue() {
        pthread_mutex_destroy(&qmutex);
        pthread_cond_destroy(&empty_condition);
    }

    void push_back(value_type& thing) {
        pthread_mutex_lock(&qmutex);
        queue.push(thing);
        assert(!queue.empty());    
        pthread_cond_signal(&empty_condition);
        std::cout << pthread_self() << " SENT SIGNAL" << std::endl;
        pthread_mutex_unlock(&qmutex);
    }

    value_type& pop_front() {
        pthread_mutex_lock(&qmutex);
        while(queue.empty()) {
            std::cout << pthread_self() << " WAIT ON EMPTY" << std::endl;
            pthread_cond_wait(&empty_condition, &qmutex);
            std::cout << pthread_self() << " RECV SIGNAL" << std::endl;
        }
        value_type& item = queue.front();
        queue.pop();
        pthread_mutex_unlock(&qmutex);
        return item;
    }
    
};

template<typename T>
void* pthread_trampoline(void* obj) {
    reinterpret_cast<T>(obj)->dispatch();
    return NULL;
}

class request_processor {
    request_queue& queue;
    pthread_t thread;

    public:
    request_processor(request_queue& queue) \
        : queue(queue) {
        pthread_create(&thread, NULL, pthread_trampoline<decltype(this)>, this);
    }

    void dispatch() {
        while(1) {
            request_queue::value_type req = queue.pop_front();
            struct evhttp_uri* req_uri = evhttp_uri_parse(evhttp_request_get_uri(req));

            std::cout << pthread_self() << " REQUEST URI: " << evhttp_request_get_uri(req) << std::endl;

            HTTP_ERROR_500(req); 
            
            evhttp_uri_free(req_uri); 
            //evhttp_request_free(req);
        }
    }

    void HTTP_ERROR_500(request_queue::value_type req) {
            struct evbuffer* resp = evbuffer_new();

            std::list<std::string> ssl_friends;
            rsPeers->getFriendList(ssl_friends);
            
            for(auto iter = ssl_friends.begin(); iter != ssl_friends.end(); ++iter) {
                evbuffer_add(resp, iter->c_str(), iter->size());
                //std::cout << "friend " << *iter << rsPeers->getPeerName(*iter) << ":" << rsPeers->isOnline(*iter) << std::endl;
            }
           
            struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
            evhttp_add_header(headers, "Content-Type", "text/plain");
            evhttp_send_reply(req, 200, "OK", resp);
            //evbuffer_free(resp);

    }
};

void queue_request(struct evhttp_request* r, void* arg) {
    std::cout << pthread_self() << " QUEUE ADD: " << arg << std::endl;
    request_queue* rq = static_cast<request_queue*>(arg);
    assert(rq);
    rq->push_back(r);
}
};
