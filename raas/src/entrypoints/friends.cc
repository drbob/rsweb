#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rsinit.h>
#include <boost/foreach.hpp>
namespace rsweb {

void ep_friends(evhttp_request* req) {
    // friends accounts
    std::list<std::string> ssl_friends;
    rsPeers->getFriendList(ssl_friends);
    auto jroot = json_object();
    for(auto iter = ssl_friends.begin(); iter != ssl_friends.end(); ++iter) {
        RsPeerDetails peer;
        rsPeers->getPeerDetails(*iter, peer);
        json_t* json_friend = serialize_RsPeerDetails_to_json(peer);
        json_object_set(jroot, iter->c_str(), json_friend);
    }

    // local accounts 
    std::list<std::string> profile_ids;
    RsInit::getAccountIds(profile_ids);
   
    BOOST_FOREACH(std::string& id, profile_ids) {
        RsPeerDetails profile;
        if(rsPeers->getPeerDetails(id, profile)) {
            json_t* profile_json = serialize_RsPeerDetails_to_json(profile);
            json_object_set_new(jroot, id.c_str(), profile_json);
        }
    }
    
    evhttp_send_json_reply(req, jroot);
}


}
