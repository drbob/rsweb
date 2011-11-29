#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
namespace rsweb {


json_t* serialize_ChatInfo_to_json(const ChatInfo& chat) {
    auto json_msg = json_object();
     
    json_object_set_new(json_msg, "from", json_string(chat.rsid.c_str()));
    json_object_set_new(json_msg, "send_time", json_integer(chat.sendTime));
    json_object_set_new(json_msg, "recv_time", json_integer(chat.recvTime));
    json_object_set_new(json_msg, "msg", json_string(wstring_to_utf8_string(chat.msg).c_str()));

    return json_msg;
}


void ep_im_chat_GET(evhttp_request* req) {
    // i have no idea what this actually does and we dont actually need to call it
    const int qcount = rsMsgs->getPublicChatQueueCount();

    std::list<std::string> active_chats;
    rsMsgs->getPrivateChatQueueIds(true, active_chats);

    auto json_chats = json_object();
    for(std::string& id : active_chats) {
        std::cout << "chat " << id << std::endl;
        
        std::list<ChatInfo> chat_queue;
        auto json_chat_messages = json_array();
        
        rsMsgs->getPrivateChatQueue(true, id, chat_queue);
        for(ChatInfo& chat : chat_queue) {
            auto json_msg = serialize_ChatInfo_to_json(chat);
            json_array_append_new(json_chat_messages, json_msg);
        }
        
        json_object_set_new(json_chats, id.c_str(), json_chat_messages);

        rsMsgs->clearPrivateChatQueue(true, id);
    }
    evhttp_send_json_reply(req, json_chats);
}

void ep_im_chat_POST(evhttp_request* req) {

}

void ep_im_chat(evhttp_request* req) {
    // send or recv?
    evhttp_cmd_type method = evhttp_request_get_command(req);
    if(method == evhttp_cmd_type::EVHTTP_REQ_GET) {
        ep_im_chat_GET(req);
    } else if(method == evhttp_cmd_type::EVHTTP_REQ_POST) {
        ep_im_chat_POST(req);
    }
}


void ep_global_chat_GET(evhttp_request* req) {
    std::list<ChatInfo> chat;
    rsMsgs->getPublicChatQueue(chat);
    
    auto json_messages = json_array();
    for(ChatInfo& msg : chat) {
        json_t* json_msg = serialize_ChatInfo_to_json(msg);
        json_array_append_new(json_messages, json_msg);
    }

    auto jroot = json_object();
    json_object_set_new(jroot, "messages", json_messages);
   
    evhttp_send_json_reply(req, jroot);
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
