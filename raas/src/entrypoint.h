#ifndef RSWEB_ENTRYPOINT_HELPER
#define RSWEB_ENTRYPOINT_HELPER

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/keyvalq_struct.h>
#include <event2/http.h>
#include <list>
#include <iostream>
#include <tuple>
#include <functional>
#include <algorithm>
#include <jansson.h>
#include <QString>
#include <boost/regex.hpp>
#include "entrypoints/enabled.h"

namespace rsweb {

// the actual ep funcs look like this
typedef std::function<void (evhttp_request*)> entrypoint_func_type;
typedef std::function<bool (const std::string&, evhttp_request*)> entrypoint_matcher_func_type;

// returns a closure that uses predicate_type to control execution of func
template <typename Pt>
struct entrypoint_matcher_template {
    typedef Pt predicate_type;

    entrypoint_matcher_func_type operator()(
            const typename predicate_type::first_argument_type match,
            const entrypoint_func_type& func)
    const {
        // the closure that actually runs the request handler
        return [=](const typename predicate_type::second_argument_type& test, 
                   const typename entrypoint_func_type::argument_type request)
        {
            auto result = predicate_type()(match, test);
            if(result) func(request);
            return result;
        };
    }
};

// the follow two types work together to implement entrypoint_rx
// with instantiation-time compiler RX instead of on each call
// of the returned closure
struct regex_match_predicate {
    typedef boost::regex first_argument_type;
    typedef std::string second_argument_type;

    bool operator()(const first_argument_type& rx, const second_argument_type& test) const {
        return boost::regex_search(test, rx);
    }
};

struct entrypoint_rx_type : public entrypoint_matcher_template<regex_match_predicate> {
    typedef entrypoint_matcher_template<regex_match_predicate> inherited;

    entrypoint_matcher_func_type operator()(
            const std::string& match,
            const entrypoint_func_type& func)
    {
        // compile the regex before storing it in the closure
        return inherited::operator()(inherited::predicate_type::first_argument_type(match), func); 
    }
};

// instances of the entrypoint matcher types for more convenient syntax
static entrypoint_matcher_template<std::equal_to<std::string>> entrypoint_eq;
static entrypoint_rx_type entrypoint_rx;

static int json_dump_evbuffer(json_t* json, evbuffer* eb, int flags=0) {
    return json_dump_callback(json,
            ([](const char* b, size_t n, void* d){
                    return evbuffer_add((evbuffer*)d, b, n);
                }),
            eb, flags);
}

static std::string& wstring_to_utf8_string(const std::wstring& src, std::string& dest) {
    return dest = std::string(QString::fromWCharArray(src.c_str(), src.size()).toUtf8().data());
}

static std::string wstring_to_utf8_string(const std::wstring& src) {
    std::string temp;
    wstring_to_utf8_string(src, temp);
    return temp;
}


static void evhttp_send_json_reply(evhttp_request* req, json_t* jroot) {
    struct evbuffer* resp = evbuffer_new();
    json_dump_evbuffer(jroot, resp, JSON_INDENT(4)); 
    json_object_clear(jroot);
    json_decref(jroot);

    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 200, "OK", resp);
    evbuffer_free(resp);
}

};

#endif
