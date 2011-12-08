#ifndef RSWEB_MIDDLEW_HELPER_H
#define RSWEB_MIDDLEW_HELPER_H
#include <functional>
#include <event2/http.h>
namespace rsweb {

typedef std::function<evhttp_request* (evhttp_request*)> middleware_func_type;

}


#endif






