#ifndef RSWEB_REQUEST_H
#define RSWEB_REQUEST_H

#include <event2/http.h>

namespace rsweb {

    
struct request {
    typedef evhttp_request request_type;
    typedef std::tr1::regex regex_type;

    request_type req;
    protected:
    struct evhttp_uri* parsed_uri;
    struct evkeyvalq* queryvars;
    struct evkeyvalq* postvars;
    struct evkeyvalq* headers;

    std::string& __get_string_from_evkvq(const std::string& name, const struct evkeyvalq* kvq) { 
        const char* val = evhttp_find_header(kvq, name.c_str());
        if(val) return std::string(val);
        else return std::string();
    }

    public:
    request(request_type& r) : 
        req(r),
        parsed_uri(evhttp_request_get_evhttp_uri(r)),
        queryvars(0),
        postvars(0),
        headers(0),
    {
    }
   
    std::string& get_postvar(const std::string& name) {
    }

    std::string& get_queryvar(const std::string& name) {
        if(!queryvars) evhttp_parse_query_str(evhttp_uri_get_query(parsed_uri), queryvars);
        return __get_string_from_evkvq(name, queryvars);
    }

    std::string& get_input_header(const std::string& name) {
        if(!headers) headers = evhttp_request_get_input_headers(req);
        return __get_string_from_evkvq(name, headers);
    }

    std::string& path() {
        return std::string(evhttp_uri_get_path(parsed_uri));
    }

};
};

#endif
