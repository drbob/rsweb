#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <boost/algorithm/string.hpp>



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
    // extract the path from the URI (drop query and fragment) and decode it
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

    std::list<std::string> active_chats;

    // pull the chat ID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    if(path_parts.size() >= 4) {
        // drop various bits off the front of the path
        path_parts.pop_front(); // ("")
        path_parts.pop_front(); // ("messages")
        path_parts.pop_front(); // ("im")
        active_chats.push_back(path_parts.front());
    }

    // if no queue specified in the URL, generate messages from all queues!
    if(active_chats.empty()) {
        rsMsgs->getPrivateChatQueueIds(true, active_chats);
        active_chats.push_back("public");
    }

    auto json_chats = json_object();
    for(std::string& id : active_chats) {
        std::list<ChatInfo> chat_queue;
        auto json_chat_messages = json_array();
        rsMsgs->getPrivateChatQueue(true, id, chat_queue);
        for(ChatInfo& chat : chat_queue) {
            auto json_msg = serialize_ChatInfo_to_json(chat);
            json_array_append_new(json_chat_messages, json_msg);
        }

        json_object_set_new(json_chats, id.c_str(), json_chat_messages);

        // rs doesn't clear this automatically so we tell it to
        rsMsgs->clearPrivateChatQueue(true, id);
    }

    // pull in public chat messages if desired
    if(std::find(active_chats.begin(), active_chats.end(),
                "public") != active_chats.end()
      ) {
        std::list<ChatInfo> chat_queue;
        rsMsgs->getPublicChatQueue(chat_queue);
        json_t* json_chat_messages = json_array();
        for(ChatInfo& chat : chat_queue) {
            // FIXME: filter out messages that we sent
            json_array_append_new(json_chat_messages, serialize_ChatInfo_to_json(chat));
        }
        json_object_set_new(json_chats, "public", json_chat_messages);
    } 

    evhttp_send_json_reply(req, json_chats);
}

void ep_im_chat_POST(evhttp_request* req) {
    // extract the path from the URI (drop query and fragment) and decode it
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

    // pull the chat ID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    // drop various bits off the front of the path
    path_parts.pop_front(); // ("")
    path_parts.pop_front(); // ("messages")
    path_parts.pop_front(); // ("im")
    std::string to_id = path_parts.front();

    // alternate call path for posting messages to global chat
    if(to_id == "public") return ep_global_chat_POST(req);

    // fetch the post body into a vector<char>
    evbuffer* postbody = evhttp_request_get_input_buffer(req);
    size_t postlen = evbuffer_get_length(postbody);
    std::vector<char> buf(postlen+1, '\0');
    evbuffer_remove(postbody, &buf.front(), postlen);

    // parse the query string and convert it to a wstring
    struct evkeyvalq head;
    evhttp_parse_query_str(&buf.front(), &head);
    const char* body_cstr = evhttp_find_header(&head, "msg");

    std::wstring msg;
    utf8_string_to_wstring(body_cstr, msg);

    // send the message
    bool sent = rsMsgs->sendPrivateChat(to_id, msg);

    auto jroot = json_object();
    json_object_set_new(jroot, "sent", sent ? json_true() : json_false());

    evhttp_send_json_reply(req, jroot);
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

void ep_im_waiting(evhttp_request* req) {
    std::list<std::string> active_chats;
    rsMsgs->getPrivateChatQueueIds(true, active_chats);

    auto json_chats = json_object();
    for(std::string& id : active_chats) {
        std::list<ChatInfo> chat_queue;
        auto json_chat_messages = json_array();
        rsMsgs->getPrivateChatQueue(true, id, chat_queue);

        json_object_set_new(json_chats, id.c_str(), json_integer(chat_queue.size()));
    }
    evhttp_send_json_reply(req, json_chats);
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

void ep_global_chat_waiting(evhttp_request* req) {
    const int qcount = rsMsgs->getPublicChatQueueCount();
    auto jroot = json_object();
    json_object_set_new(jroot, "waiting", json_integer(qcount));
    evhttp_send_json_reply(req, jroot);
}

}
