#include "../entrypoint.h"
#include "../raas.h"

#include <list>
#include <string>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#include <retroshare/rsiface.h> 
#include <retroshare/rsinit.h>
#include <util/rsrandom.h>
#include <pqi/authgpg.h>  

namespace rsweb {

    void ep_profile_list(evhttp_request* req) {
        // grab a list of all local accounts 
        std::list<std::string> profile_ids;
        RsInit::getAccountIds(profile_ids);

        std::string preferred_id;
        RsInit::getPreferedAccountId(preferred_id);

        // Locations

        auto json_profiles = json_object();
        BOOST_FOREACH(std::string& id, profile_ids) {
            std::string gpgId, gpgName, gpgEmail, sslName;
            // we have to use this instead of rsPeers because
            // this page needs to be safe prior to StartupRetroshare() being called
            if (RsInit::getAccountDetails(id, gpgId, gpgName, gpgEmail, sslName)) {
                auto profile_json = json_object();

                json_object_set_new(profile_json, "id", json_string(id.c_str())); 
                json_object_set_new(profile_json, "pgp_id", json_string(gpgId.c_str()));
                json_object_set_new(profile_json, "name", json_string(gpgName.c_str()));
                json_object_set_new(profile_json, "email", json_string(gpgEmail.c_str()));
                json_object_set_new(profile_json, "location", json_string(sslName.c_str()));
                json_object_set_new(profile_json, "preferred", id == preferred_id ? json_true() : json_false());
                json_object_set_new(json_profiles, id.c_str(), profile_json);
            }
        }

        profile_ids.clear();

        // Local PGP identities
        json_t* pgp_certs = json_object();
        std::list<std::string>& pgp_ids = profile_ids; // reuse the list
        RsInit::GetPGPLogins(pgp_ids);
        BOOST_FOREACH(std::string& id, pgp_ids) {
            RsPeerDetails gpg;
            if(AuthGPG::getAuthGPG()->getGPGDetails(id, gpg)) {
                gpg.isOnlyGPGdetail = true;
                json_t* gpg_j = serialize_RsPeerDetails_to_json(gpg);
                json_object_set_new(pgp_certs, id.c_str(),  gpg_j);
            }
        }

        json_t* jroot = json_object();
        json_object_set_new(jroot, "identities", pgp_certs);
        json_object_set_new(jroot, "profiles", json_profiles);

        evhttp_send_json_reply(req, jroot);
    }

    void ep_profile_create(evhttp_request* req) {
        struct evkeyvalq vars;
        evhttp_parse_form_urlencoded_post(req, &vars);

        // define expected vars
        std::map<const char*, std::string> form_vars = { 
            {"name", ""},
            {"pgp_id", ""}
        };

        // read them out of libevent
        BOOST_FOREACH(const char* key, form_vars | boost::adaptors::map_keys) {
            const char* c_value = evhttp_find_header(&vars, key);
            form_vars[key] = c_value ? c_value : "";
        }

        // try and generate the new location profile
        const std::string sslPasswd = RSRandom::random_alphaNumericString(RsInit::getSslPwdLen()) ;
        RsInit::SelectGPGAccount(form_vars["pgp_id"]);

        std::string err, sslId;
        bool okGen = RsInit::GenerateSSLCertificate(form_vars["pgp_id"], "", form_vars["name"], "", sslPasswd, sslId, err);

        json_t* jroot = json_object();

        if(okGen) {
            json_object_set_new(jroot, "id", json_string(sslId.c_str()));
        } else {
            json_object_set_new(jroot, "error", json_string(err.c_str()));
        }

        evhttp_send_json_reply(req, jroot);
    }

    void ep_pgp_identity_create(evhttp_request* req) {
        // parse the form data
        struct evkeyvalq vars;
        evhttp_parse_form_urlencoded_post(req, &vars);

        // define expected vars
        std::map<const char*, std::string> form_vars = { 
            {"name", "HERF"},
            {"email", "DURF"},
            {"password", "DERP"}
        };

        // read them out of libevent
        BOOST_FOREACH(const char* key, form_vars | boost::adaptors::map_keys) {
            const char* c_value = evhttp_find_header(&vars, key);
            std::cerr << key << " = " << form_vars[key] << " = '" << c_value <<  "'" << std::endl;
            form_vars[key] = std::string(c_value ? c_value : "");
        }

        std::cerr << form_vars["name"] << " / " << form_vars["email"] << " / " << form_vars["password"] << std::endl;

        // try and generate the pgp key
        std::string pgp_id, error;
        bool genret = RsInit::GeneratePGPCertificate(
                form_vars["name"],
                form_vars["email"],
                form_vars["password"],
                pgp_id, error);

        json_t* jroot = json_object();
        if(genret) {
            json_object_set_new(jroot, "id", json_string(pgp_id.c_str()));
        } else {
            json_object_set_new(jroot, "error", json_string(error.c_str()));
        }

        evhttp_send_json_reply(req, jroot); 
    }

    void ep_profile_active(evhttp_request* req) {
        RsPeerDetails you;
        rsPeers->getPeerDetails(rsPeers->getOwnId(), you);
        evhttp_send_json_reply(req, serialize_RsPeerDetails_to_json(you));
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
}
