#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
namespace rsweb {

void ep_global_chat_GET(evhttp_request* req) {
    std::list<ChatInfo> chat;
    rsMsgs->getPublicChatQueue(chat);
    auto jroot = json_object();
    auto json_messages = json_array();
    json_object_set_new(jroot, "messages", json_messages);

    for(auto iter = chat.begin(); iter != chat.end(); ++iter) {
        std::cout << iter->rsid << std::endl; //" " << iter->msg << std::endl;

        auto json_msg = json_object();
        // make a coroutine to set the keys to reduce C&P
        // really need a nice C++ified wrapper for the JSON lib
        auto set_string = [&](const char* key, std::string str){
            json_object_set_new(json_msg, key, json_string(str.c_str()));
        };
        auto set_int = [&](const char* key, uint32_t i){
            json_object_set_new(json_msg, key, json_integer(i));
        };

        set_string("from", iter->rsid);
        set_int("send_time", iter->sendTime);
        set_int("recv_time", iter->recvTime);
        // this is horrendously ugly but at least QT is doing something useful
        set_string("msg",
                std::string(QString::fromWCharArray(iter->msg.c_str(), iter->msg.size()).toUtf8().data())
                );
    
        json_array_append_new(json_messages, json_msg);
    }

    struct evbuffer* resp = evbuffer_new();
    json_dump_evbuffer(jroot, resp, JSON_INDENT(4)); 
    json_object_clear(jroot);
    json_decref(jroot);
    
    struct evkeyvalq* headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "application/json");
    evhttp_send_reply(req, 200, "OK", resp);
    evbuffer_free(resp);
}

void ep_global_chat_POST(evhttp_request* req) {
    // libevent has somewhat handicapped http POST input handling
    // it has already received the entire body into a buffer when we get here
    // while what we really want is to be able to attach a cb to incoming
    // data or at least for completion

    // fetch the post body into a vector<char>
    evbuffer* postbody = evhttp_request_get_input_buffer(req);
    size_t postlen = evbuffer_get_length(postbody);
    std::vector<char> buf(postlen+1, '\0');
    evbuffer_remove(postbody, &buf.front(), postlen);

    // parse the query string and convert it to a wstring
    struct evkeyvalq head;
    evhttp_parse_query_str(&buf.front(), &head);
    const char* msgdata = evhttp_find_header(&head, "msg");

    std::wstring msg(QString::fromUtf8(msgdata).toStdWString());
    
    rsMsgs->sendPublicChat(msg);
    
    evhttp_send_reply(req, 200, "OK", NULL);
}

void ep_global_chat(evhttp_request* req) {
    // send or recv?
    evhttp_cmd_type method = evhttp_request_get_command(req);
    if(method == evhttp_cmd_type::EVHTTP_REQ_GET) {
        ep_global_chat_GET(req);
    } else if(method == evhttp_cmd_type::EVHTTP_REQ_POST) {
        ep_global_chat_POST(req);
    }
}

};
