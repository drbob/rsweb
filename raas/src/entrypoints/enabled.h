#ifndef RSWEB_ENTRYPOINTS_ENABLED
#define RSWEB_ENTRYPOINTS_ENABLED
namespace rsweb {
void ep_friends(evhttp_request*);
void ep_global_chat(evhttp_request*);
void ep_static_files(evhttp_request*);
void ep_file_share_browse(evhttp_request*);
void ep_forum_index(evhttp_request*);
void ep_forum_thread(evhttp_request*);

static void ep_http_404(evhttp_request* req) {
    struct evbuffer* resp = evbuffer_new();
    evbuffer_add_printf(resp, "404 :(\n%s", evhttp_request_get_uri(req)); 

    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 404, "Not Found", resp);
    evbuffer_free(resp);
}


};
#endif
