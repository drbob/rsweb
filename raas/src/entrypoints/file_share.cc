#include "file_share.h"
#include "../entrypoint.h"
#include "../http_errors.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsfiles.h>
#include <retroshare/rstypes.h>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
namespace rsweb {


// /file_sharing/<uid>/ -- list the root of that users files
// /file_sharing/<uid>/
void ep_file_share_browse(evhttp_request* req) {
    // extract the path from the URI (drop query and fragment) and decode it
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

    // pull the UID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    // drop various bits off the front of the path
    path_parts.pop_front(); // ("")
    path_parts.pop_front(); // ("file_share")
    std::string uid = path_parts.front();
    path_parts.pop_front(); // (uid)
   
    std::string new_path = boost::join(path_parts, "/");

    DirDetails dir;
    void* ref = NULL;

    // get the root for this uid
    rsFiles->RequestDirDetails(uid, "", dir);
    
    // walk down the path stack to the desired dir
    BOOST_FOREACH(std::string& part, path_parts) {
        // find the desired ref at this depth
        rsFiles->RequestDirDetails(ref, dir, 0);
        ref = NULL;
        BOOST_FOREACH(DirStub& s, dir.children) {
            if(s.name == part) {
                ref = s.ref;
                break;
            }
        }
        // if we dont find ref then the path is wrong
        if(!ref) break;
    }

    if(!ref) return ep_http_404(req);

    uint32_t ref_type = rsFiles->getType(ref, 0);
    assert(!(ref_type & DIR_TYPE_DIR && ref_type & DIR_TYPE_FILE));

    // if the path terminated on a dir then s.type & DIR_TYPE_DIR == true
    // and we'll need to do one more RequestDirDetails to get the content we want
    if(ref_type & DIR_TYPE_DIR) {
        auto jroot = json_object();
        json_object_set_new(jroot, "path", json_string(new_path.c_str()));
        return __output_DirDetails_DIR_as_json(req, ref, jroot); 
    }
    else if(ref_type & DIR_TYPE_FILE) {
        return __output_DirDetails_FILE_data(req, ref);
    }
}

void __output_DirDetails_FILE_data(evhttp_request* req, void* ref) {
    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 200, "OK", NULL);
}

void __output_DirDetails_DIR_as_json(evhttp_request* req, void* ref, json_t* jroot) { 
    // now build a json object for this folder view
    DirDetails dir;
    rsFiles->RequestDirDetails(ref, dir, 0);
    auto json_folder_list = json_array();
    BOOST_FOREACH(DirStub& s, dir.children) {
        auto json_dentry = json_object();
        json_object_set_new(json_dentry, "name", json_string(s.name.c_str()));
        json_object_set_new(json_dentry, "type", json_integer(s.type));
        json_array_append_new(json_folder_list, json_dentry);
    }

    json_object_set_new(jroot, "list", json_folder_list);

    // dump the json out
    struct evbuffer* resp = evbuffer_new();
    json_dump_evbuffer(jroot, resp, JSON_INDENT(4)); 
    json_object_clear(jroot);
    json_decref(jroot);

    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, 200, "OK", resp);
    evbuffer_free(resp);
}

void __map_rsweb_url_to_local_file(std::string url, std::string& file_url) {

}

};
 
