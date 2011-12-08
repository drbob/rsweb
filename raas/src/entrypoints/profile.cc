#include "../entrypoint.h"
#include <retroshare/rsiface.h> 
#include <retroshare/rsinit.h>  
#include <list>
#include <string>
#include <boost/foreach.hpp>
#include "../raas.h"


namespace rsweb {

void ep_profile_list(evhttp_request* req) {
    // grab a list of all local accounts 
    std::list<std::string> profile_ids;
    RsInit::getAccountIds(profile_ids);

    std::string preferred_id;
    RsInit::getPreferedAccountId(preferred_id);
    
    auto json_profiles = json_object();
    BOOST_FOREACH(std::string& id, profile_ids) {
        std::string gpgId, gpgName, gpgEmail, sslName;
        if (RsInit::getAccountDetails(id, gpgId, gpgName, gpgEmail, sslName)) {
            auto profile_json = json_object();
            json_object_set_new(profile_json, "id", json_string(id.c_str())); 
            json_object_set_new(profile_json, "gpg_id", json_string(gpgId.c_str()));
            json_object_set_new(profile_json, "gpg_name", json_string(gpgName.c_str()));
            json_object_set_new(profile_json, "gpg_email", json_string(gpgEmail.c_str()));
            json_object_set_new(profile_json, "ssl_name", json_string(sslName.c_str()));
            json_object_set_new(profile_json, "preferred", id == preferred_id ? json_true() : json_false());
            json_object_set_new(json_profiles, id.c_str(), profile_json);
        }
    }

    evhttp_send_json_reply(req, json_profiles);
}

void ep_profile_activate(evhttp_request* req) {
    // account activation is done entirely via POST
 
    // fetch the post body into a vector<char>
    evbuffer* postbody = evhttp_request_get_input_buffer(req);
    size_t postlen = evbuffer_get_length(postbody);
    std::vector<char> buf(postlen+1, '\0');
    evbuffer_remove(postbody, &buf.front(), postlen);
 
    // parse the posted query string and convert it to a wstring
    struct evkeyvalq head;
    evhttp_parse_query_str(&buf.front(), &head);
    const char* profile_cstr = evhttp_find_header(&head, "profile");
    std::string acc_id = profile_cstr ? profile_cstr : "";
    std::string errstr = "Malformed activation request or profile already active";

    if(rsweb::rs_control_startup_retroshare_called == false && !acc_id.empty()) {
        std::string gpgId, gpgName, gpgEmail, sslName;
        if (RsInit::getAccountDetails(acc_id, gpgId, gpgName, gpgEmail, sslName)) {
            RsInit::SelectGPGAccount(gpgId);
        }

        std::string error_string;
        int retVal = RsInit::LockAndLoadCertificates(false, error_string);
        if(retVal == 1) {
            errstr = "Another instance of retroshare is already using this profile";
        } else if(retVal == 2) {
            errstr = "An unexpected error occurred while locking the profile";
        } else if(retVal == 2) {
            errstr = "An error occurred while login with the profile";
        } else if(retVal != 0) {
            errstr = "Unknown error while loading certificates";
        } else {
            errstr = "";
        }

        rsweb::rs_control->StartupRetroShare();
        rsweb::rs_control_startup_retroshare_called = true;
    }

    auto jroot = json_object();
    if(errstr.empty()) json_object_set_new(jroot, "activated", json_string(acc_id.c_str()));
    else json_object_set_new(jroot, "error", json_string(errstr.c_str()));
    evhttp_send_json_reply(req, jroot);
}

};

