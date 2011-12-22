#include "../entrypoint.h"
#include "../http_errors.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
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
    std::cout << "loading file " << fs_path.string() << std::endl;

    // let libev know we want to send this file
    int file_fd = open(fs_path.string().c_str(), O_RDONLY);
    if(file_fd == -1) return rsweb::ep_http_404(req);

    struct evbuffer* outbuf = evhttp_request_get_output_buffer(req);
    evbuffer_add_file(outbuf, file_fd, 0, fs::file_size(fs_path)); 

    const std::string filename = fs_path.filename().string();
    const char* mime = "text/plain";
    if(boost::ends_with(filename, ".js")) mime = "application/ecmascript";
    else if(boost::ends_with(filename, ".html")) mime = "text/html";
    else if(boost::ends_with(filename, ".css")) mime = "text/css";

    // set the http response
    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", mime);
    evhttp_send_reply(req, 200, "OK", NULL);


}
};
