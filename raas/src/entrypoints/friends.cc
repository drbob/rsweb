#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
namespace rsweb {

void ep_friends(evhttp_request* req) {
    std::list<std::string> ssl_friends;
    rsPeers->getFriendList(ssl_friends);

    auto jroot = json_object();
    for(auto iter = ssl_friends.begin(); iter != ssl_friends.end(); ++iter) {
        auto json_friend = json_object();
        RsPeerDetails peer;
        rsPeers->getPeerDetails(*iter, peer);
        // make a coroutine to set the keys to reduce C&P
        auto set_string = [&](const char* key, std::string str){
            json_object_set_new(json_friend, key, json_string(str.c_str()));
        };
        auto set_int = [&](const char* key, uint32_t i){
            json_object_set_new(json_friend, key, json_integer(i));
        };


        set_string("id", peer.id);
        set_string("gpg_id", peer.gpg_id);
        set_string("name", peer.name);
        set_string("email", peer.email);
        set_string("location", peer.location);
        set_string("org", peer.org);
        set_int("connect_state", peer.connectState);
        json_object_set(jroot, iter->c_str(), json_friend);
    }

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
