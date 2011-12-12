#ifndef RSWEB_ENTRYPOINT_HELPER
#define RSWEB_ENTRYPOINT_HELPER

#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/keyvalq_struct.h>
#include <event2/http.h>
#include <list>
#include <iostream>
#include <tuple>
#include <functional>
#include <algorithm>
#include <jansson.h>
#include <QString>
#include <boost/regex.hpp>


#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsstatus.h>

#include "entrypoints/enabled.h"

namespace rsweb {

    // the actual ep funcs look like this
    typedef std::function<void (evhttp_request*)> entrypoint_func_type;
    typedef std::function<bool (const std::string&, evhttp_request*)> entrypoint_matcher_func_type;

    // returns a closure that uses predicate_type to control execution of func
    template <typename Pt>
        struct entrypoint_matcher_template {
            typedef Pt predicate_type;

            entrypoint_matcher_func_type operator()(
                    const typename predicate_type::first_argument_type match,
                    const entrypoint_func_type& func)
                const {
                    //FIXME: this provides no way to defer calling the entrypoint
                    // instead of returning a naive closure we should return a more complex functor
                    // that gives us access to information about the predicate being used

                    // the closure that actually runs the request handler
                    return [=](const typename predicate_type::second_argument_type& test, 
                            const typename entrypoint_func_type::argument_type request) -> typename predicate_type::return_type
                    {
                        auto result = predicate_type()(match, test);
                        if(result) func(request);
                        return result;
                    };
                }
        };

    // the follow two types work together to implement entrypoint_rx
    // with instantiation-time compiler RX instead of on each call
    // of the returned closure
    struct regex_match_predicate {
        typedef boost::regex first_argument_type;
        typedef std::string second_argument_type;
        typedef bool return_type;

        bool operator()(const first_argument_type& rx, const second_argument_type& test) const {
            return boost::regex_search(test, rx);
        }
    };

    struct entrypoint_rx_type : public entrypoint_matcher_template<regex_match_predicate> {
        typedef entrypoint_matcher_template<regex_match_predicate> inherited;

        entrypoint_matcher_func_type operator()(
                const std::string& match,
                const entrypoint_func_type& func)
        {
            // compile the regex before storing it in the closure
            return inherited::operator()(inherited::predicate_type::first_argument_type(match), func); 
        }
    };

    // instances of the entrypoint matcher types for more convenient syntax
    static entrypoint_matcher_template<std::equal_to<std::string>> entrypoint_eq;
    static entrypoint_rx_type entrypoint_rx;

    static int json_dump_evbuffer(json_t* json, evbuffer* eb, int flags=0) {
        return json_dump_callback(json,
                ([](const char* b, size_t n, void* d){
                 return evbuffer_add((evbuffer*)d, b, n);
                 }),
                eb, flags);
    }
    
    static std::string& wstring_to_utf8_string(const std::wstring& src, std::string& dest) {
        return dest = std::string(QString::fromWCharArray(src.c_str(), src.size()).toUtf8().data());
    }

    static std::string wstring_to_utf8_string(const std::wstring& src) {
        std::string temp;
        wstring_to_utf8_string(src, temp);
        return temp;
    }

    static std::wstring& utf8_string_to_wstring(const std::string& src, std::wstring& dest) {
        dest = QString::fromUtf8(src.c_str(), src.length()).toStdWString();
        return dest;
    }

    static std::wstring utf8_string_to_wstring(const std::string& src) {
        return QString::fromUtf8(src.c_str(), src.length()).toStdWString();
    }


    static json_t* serialize_ChatInfo_to_json(const ChatInfo& chat) {
        auto json_msg = json_object();

        json_object_set_new(json_msg, "from", json_string(chat.rsid.c_str()));
        json_object_set_new(json_msg, "send_time", json_integer(chat.sendTime));
        json_object_set_new(json_msg, "recv_time", json_integer(chat.recvTime));
        json_object_set_new(json_msg, "msg", json_string(wstring_to_utf8_string(chat.msg).c_str()));

        return json_msg;
    }

    static json_t* serialize_RsPeerDetails_to_json(RsPeerDetails& peer) {
        json_t* json_friend = json_object();

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
        set_string("fingerprint", peer.fpr);
        set_int("trust_level", peer.trustLvl);
        set_int("valid_level", peer.validLvl);

        if(peer.isOnlyGPGdetail == false) {
            set_string("org", peer.org);
            set_string("location", peer.location);
            set_int("connect_state", peer.connectState);

            // grab the users presence setting if it's available
            StatusInfo peer_status;
            if(rsStatus && rsStatus->getStatus(peer.id, peer_status)) {
                set_int("status", peer_status.status); 
            }
        }

        return json_friend;
    }

    static void evhttp_send_json_reply(evhttp_request* req, json_t* jroot) {
        struct evbuffer* resp = evbuffer_new();
        json_dump_evbuffer(jroot, resp, JSON_INDENT(4)); 
        json_object_clear(jroot);
        json_decref(jroot);

        struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(headers, "Content-Type", "text/plain");
        evhttp_send_reply(req, 200, "OK", resp);
        evbuffer_free(resp);
    }

    static int evhttp_parse_form_urlencoded_post(evhttp_request* req, struct evkeyvalq* vars) {
        // fetch the post body into a vector<char>
        evbuffer* postbody = evhttp_request_get_input_buffer(req);
        size_t postlen = evbuffer_get_length(postbody);
        std::vector<char> buf(postlen+1, '\0');
        evbuffer_remove(postbody, &buf.front(), postlen);
        return evhttp_parse_query_str(&buf.front(), vars);
    }
}
#endif
