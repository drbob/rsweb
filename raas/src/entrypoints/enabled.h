#ifndef RSWEB_ENTRYPOINTS_ENABLED
#define RSWEB_ENTRYPOINTS_ENABLED

namespace rsweb {
    void ep_static_files(evhttp_request*);

    void ep_friends(evhttp_request*);
    void ep_friend_edit(evhttp_request*);
    void ep_friend_add(evhttp_request*);

    void ep_global_chat(evhttp_request*);
    void ep_global_chat_POST(evhttp_request*);
    void ep_global_chat_waiting(evhttp_request*);

    void ep_im_chat(evhttp_request*);
    void ep_im_waiting(evhttp_request* req);

    void ep_file_share_browse(evhttp_request*);

    void ep_forum_index(evhttp_request*);
    void ep_forum_thread(evhttp_request*);
    void ep_forum_create(evhttp_request*);

    void ep_profile_list(evhttp_request*);
    void ep_profile_activate(evhttp_request*);
    void ep_profile_active(evhttp_request*);
    void ep_profile_create(evhttp_request*);
    void ep_pgp_identity_create(evhttp_request*);
}
#endif
