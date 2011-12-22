#include <event2/buffer.h>
#include <event2/http.h>

namespace rsweb {
    void ep_http_500(evhttp_request* req) {
        struct evbuffer* resp = evbuffer_new();
        evbuffer_add_printf(resp, "500 :(\n%s", evhttp_request_get_uri(req)); 

        struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(headers, "Content-Type", "text/plain");
        evhttp_send_reply(req, 500, "Internal Server Error", resp);
        evbuffer_free(resp);
    }

    void ep_http_404(evhttp_request* req) {
        struct evbuffer* resp = evbuffer_new();
        evbuffer_add_printf(resp, "404 :(\n%s", evhttp_request_get_uri(req)); 

        struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(headers, "Content-Type", "text/plain");
        evhttp_send_reply(req, 404, "Not Found", resp);
        evbuffer_free(resp);
    }
}
