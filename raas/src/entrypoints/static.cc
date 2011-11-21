#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <boost/filesystem.hpp>
#include <fcntl.h>
#include <sys/stat.h>

namespace rsweb {

namespace fs = boost::filesystem;

void ep_static_files(evhttp_request* req) {
    // map URI path to local FS
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    const char* uri_path = evhttp_uri_get_path(uri);
    fs::path fs_path = fs::current_path() / fs::path(uri_path);
  
    // FIXME: check that uri_path resides within the desired
    // sub-tree

    // let libev know we want to send this file
    int file_fd = open(fs_path.string().c_str(), O_RDONLY);
    if(file_fd == -1) return rsweb::ep_http_404(req);

    struct evbuffer* outbuf = evhttp_request_get_output_buffer(req);
    evbuffer_add_file(outbuf, file_fd, 0, fs::file_size(fs_path)); 

    // set the http response
    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 200, "OK", NULL);

}
};
