#include "../entrypoint.h"
#include "../http_errors.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rsinit.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
namespace rsweb {
void ep_friends(evhttp_request* req) {
    // friends accounts
    std::list<std::string> ssl_friends;
    rsPeers->getFriendList(ssl_friends);
    auto jroot = json_object();
    BOOST_FOREACH(auto ssl_id, ssl_friends) {
        RsPeerDetails peer;
        rsPeers->getPeerDetails(ssl_id, peer);
        json_t* json_friend = serialize_RsPeerDetails_to_json(peer);
        json_object_set(jroot, ssl_id.c_str(), json_friend);
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

void ep_friend_add(evhttp_request* req) {
    struct evkeyvalq vars;
    evhttp_parse_form_urlencoded_post(req, &vars);

    const char* cert_data = evhttp_find_header(&vars, "cert");
    if(cert_data) {
        RsPeerDetails new_peer;
        std::string errors;
       
        if(rsPeers->loadDetailsFromStringCert(std::string(cert_data),
                    new_peer,
                    errors)
          ) {
            // automatically give the user some trust
            rsPeers->trustGPGCertificate(new_peer.gpg_id, RS_TRUST_LVL_MARGINAL);
            new_peer.trustLvl = RS_TRUST_LVL_MARGINAL;

            return evhttp_send_json_reply(req, 
                    serialize_RsPeerDetails_to_json(new_peer));
        }
    }

    return ep_http_500(req);
}

void ep_friend_edit(evhttp_request* req) {
    struct evkeyvalq vars;
    evhttp_parse_form_urlencoded_post(req, &vars);
    const char* gpg_id_cstr = evhttp_find_header(&vars, "identity");
    const char* trust_lvl = evhttp_find_header(&vars, "trust_lvl");
    const char* sign = evhttp_find_header(&vars, "sign");
    if(gpg_id_cstr) {
        std::string gpg_id(gpg_id_cstr);
        if(trust_lvl) {
            const uint32_t trust_int = boost::lexical_cast<uint32_t>(trust_lvl);
            if(trust_int >= RS_TRUST_LVL_NONE &&
                    trust_int <= RS_TRUST_LVL_ULTIMATE) {
                rsPeers->trustGPGCertificate(gpg_id, trust_int);
            }
        }

        if(sign && std::string(sign) == "yes") {
            rsPeers->signGPGCertificate(gpg_id);
        }
    
        RsPeerDetails peer;
        rsPeers->getPeerDetails(gpg_id, peer);
        return evhttp_send_json_reply(req, serialize_RsPeerDetails_to_json(peer));
    }

    return ep_http_500(req);
}
}
