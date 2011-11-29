#include "file_share.h"
#include "../entrypoint.h"
#include <retroshare/rspeers.h>
#include <retroshare/rsforums.h>
#include <boost/algorithm/string.hpp>
namespace rsweb {

void ep_forum_index(evhttp_request* req) {
    std::list<ForumInfo> forum_list;
    rsForums->getForumList(forum_list);
    
    auto json_forum_list = json_array();
    for(ForumInfo& f : forum_list) {
        auto forum_json = json_object();
        json_object_set_new(forum_json, "id", json_string(f.forumId.c_str()));
        json_object_set_new(forum_json, "flags", json_integer(f.forumFlags));
        json_object_set_new(forum_json, "subscription_flags", json_integer(f.subscribeFlags));
        json_object_set_new(forum_json, "last_post", json_integer(f.lastPost));
        json_object_set_new(forum_json, "pop", json_integer(f.pop));

        // TODO: decode to utf-8
        //json_object_set_new(forum_json, "name", f.forumName.c_str());
        //json_object_set_new(forum_json, "description", f.forumDesc.c_str());
        
        json_array_append_new(json_forum_list, forum_json);
    }

    auto jroot = json_object();
    json_object_set_new(jroot, "forums", json_forum_list);

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

void ep_forum_thread_GET(evhttp_request* req) {
    // extract the path from the URI (drop query and fragment) and decode it
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

    // pull the UID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    // drop various bits off the front of the path
    path_parts.pop_front(); // ("")
    path_parts.pop_front(); // ("forum")
    std::string forum_id = path_parts.front();
   
    // pull out the other arguments we use from the query string
    evkeyvalq queryvars;
    evhttp_parse_query_str(evhttp_uri_get_query(uri), &queryvars);
    const char* parent_cstr = evhttp_find_header(&queryvars, "parent");
    const char* down_cstr = evhttp_find_header(&queryvars, "down");
    std::string root_parent_id(parent_cstr ? parent_cstr : ""); 
    int traverse_depth = 1;
    if(down_cstr) traverse_depth = strtol(down_cstr, NULL, 10);
    if(errno != 0) traverse_depth = 1;
      
    //std::cout << "forum=" << forum_id << std::endl;
    //std::cout << "parent=" << root_parent_id << std::endl;
    //std::cout << "depth=" << traverse_depth << std::endl;

    std::list<ThreadInfoSummary> root_threads;

    rsForums->getForumThreadMsgList(forum_id, root_parent_id, root_threads);
    
    if(root_threads.empty()) return ep_http_404(req);

    // do a breadth first decent of the tree to recruse_depth leaves
    // re-use the root_threads list and just track the completed range with iterators
    // while also building our json structure
    std::list<ThreadInfoSummary>::iterator start = root_threads.begin();
    std::list<ThreadInfoSummary>::iterator stop = --root_threads.end();

    auto jroot = json_object();
    // walk the un-fetched list of messages
    for(int depth = 0; depth != traverse_depth; ++depth) {
        for(;;++start) {
            ThreadInfoSummary& m = *start;

            //std::cout << depth << " " << m.msgId << " parent=" << m.parentId << std::endl;

            auto json_msg = json_object();
            json_object_set_new(json_msg, "id", json_string(m.msgId.c_str()));
            json_object_set_new(json_msg, "parent", json_string(m.parentId.c_str()));
            json_object_set_new(json_msg, "thread", json_string(m.threadId.c_str()));
            json_object_set_new(json_msg, "forum", json_string(m.forumId.c_str()));
            json_object_set_new(json_msg, "flags", json_integer(m.msgflags));
            json_object_set_new(json_msg, "body", json_string(wstring_to_utf8_string(m.msg).c_str()));
            json_object_set_new(json_msg, "subject", json_string(wstring_to_utf8_string(m.title).c_str()));
            json_object_set_new(json_msg, "timestamp", json_integer(m.ts));
            json_object_set_new(json_msg, "child_max_ts", json_integer(m.childTS));
            json_object_set_new(jroot, m.msgId.c_str(), json_msg);

            rsForums->getForumThreadMsgList(forum_id, m.msgId, root_threads);
            // .end() will move as items are inserted
            // so we track .end() - 1 and terminate once we've processed that item
            if(start == stop) break;
        }

        if(stop == --root_threads.end()) break;

        // move the start and end pointers to the new set of data
        start = ++stop;
        stop = --root_threads.end();
    }

    return evhttp_send_json_reply(req, jroot);
}


void ep_forum_thread_POST(evhttp_request* req) {
    // extract the path from the URI (drop query and fragment) and decode it
    const struct evhttp_uri* uri = evhttp_request_get_evhttp_uri(req);
    std::string path(evhttp_uridecode(evhttp_uri_get_path(uri), 0, NULL));

    // pull the UID out on it's own because we need to special case
    // it as part of the path stack
    // FIXME: note the root URL structure is implied here. this is bad. we should
    // be able to eliminate this with the help of url routers output
    // but that requires a smarter request object
    std::list<std::string> path_parts;
    boost::split(path_parts, path, boost::is_any_of("/"));
    // drop various bits off the front of the path
    path_parts.pop_front(); // ("")
    path_parts.pop_front(); // ("forum")
    std::string forum_id = path_parts.front();

    // fetch the post body into a vector<char>
    evbuffer* postbody = evhttp_request_get_input_buffer(req);
    size_t postlen = evbuffer_get_length(postbody);
    std::vector<char> buf(postlen+1, '\0');
    evbuffer_remove(postbody, &buf.front(), postlen);

    // parse the query string and convert it to a wstring
    struct evkeyvalq head;
    evhttp_parse_query_str(&buf.front(), &head);
    const char* body_cstr = evhttp_find_header(&head, "body");
    const char* subject_cstr = evhttp_find_header(&head, "subject");
    const char* parent_id_cstr = evhttp_find_header(&head, "parent");

    // FIXME: return error about the message structure
    assert(subject_cstr && body_cstr && parent_id_cstr);

    // expand the utf8 out into wchars
    std::wstring body_wstr(QString::fromUtf8(body_cstr).toStdWString());
    std::wstring subject_wstr(QString::fromUtf8(subject_cstr).toStdWString());

    // build the message object to send out
    ForumMsgInfo new_message;
    new_message.forumId = forum_id;
    new_message.parentId = parent_id_cstr;
    new_message.title = subject_wstr;
    new_message.msg = body_wstr;

    // push the message over to RS
    if(rsForums->ForumMessageSend(new_message)) {
        auto jroot = json_object();
        json_object_set_new(jroot, "new_message_id", json_string(new_message.msgId.c_str()));
        evhttp_send_json_reply(req, jroot);
    }
    else
    {
        // FIXME: use a sensible http return value
        return ep_http_500(req);
    }
}

void ep_forum_thread(evhttp_request* req) {
    evhttp_cmd_type method = evhttp_request_get_command(req);
    if(method == evhttp_cmd_type::EVHTTP_REQ_GET) {
        ep_forum_thread_GET(req);
    } else if(method == evhttp_cmd_type::EVHTTP_REQ_POST) {
        ep_forum_thread_POST(req);
    }
}



};
