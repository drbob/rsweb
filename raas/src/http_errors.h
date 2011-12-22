#ifndef RSWEB_HTTP_ERRORS_H
#define RSWEB_HTTP_ERRORS_H

namespace rsweb {
    void ep_http_500(evhttp_request*);
    void ep_http_404(evhttp_request*);
}

#endif
