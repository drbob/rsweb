#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsfiles.h>
#include <boost/algorithm/string.hpp>
namespace rsweb {


// /file_sharing/<uid>/ -- list the root of that users files
// /file_sharing/<uid>/
void ep_file_share_browse(evhttp_request* req) {
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uri_get_path(uri));

    // pull the UID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    path_parts.erase(path_parts.begin()); // drop the start of the URL
    path_parts.erase(path_parts.begin());
    std::string uid = path_parts.front();
   
    std::cout << "uid " << uid << std::endl;

    // we also need their human-name because the current API is sucky
    std::string peername = rsPeers->getPeerName(uid);
    std::cout << "name " << peername << std::endl;
    // replace the uid with the name in the path stack
    path_parts.erase(path_parts.begin()); // drop the UID
    path_parts.insert(path_parts.begin(), peername);

    // glue the path back together for output in the JSON later
    std::string new_path = boost::join(path_parts, "/");

    DirDetails dir;
    void* ref = NULL; 
    // walk down the path stack to the desired dir list
    for(std::string& part : path_parts) {
        // find the desired ref at this depth
        rsFiles->RequestDirDetails(ref, dir, 0);
        for(DirStub& s : dir.children) {
            std::cout << part << " == " << s.name << std::endl; 
            if(s.name == part) {
                ref = s.ref;
                break;
            }
        }
    }
   
    // FIXME: sensible error on 404
    assert(ref);

    auto jroot = json_object();
    json_object_set_new(jroot, "path", json_string(new_path.c_str()));

    // now build a json object for this folder view
    rsFiles->RequestDirDetails(ref, dir, 0);
    auto json_folder_list = json_array();
    for(DirStub& s : dir.children) {
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


};
 
