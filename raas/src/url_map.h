#ifndef RSWEB_RAAS_ENTRY
#define RSWEB_RAAS_ENTRY

#include <vector>
#include <string>
#include <tuple>
#include <functional>

#include "entrypoints/enabled.h"
#include "entrypoint.h"

// This file defines the URLs that are enabled in the program
// Comparisons are made in the order they are defined in this file.
// Be careful to get the ordering right.

namespace rsweb {
    //FIXME: really this should be extern
    std::vector<entrypoint_matcher_func_type> entrypoints = {
        entrypoint_rx("^/static/.*",
                ep_static_files),

        entrypoint_rx("^/messages/global_chat$",
                ep_global_chat),

        entrypoint_rx("^/messages/global_chat/waiting$",
                ep_global_chat_waiting),

        entrypoint_rx("^/messages/im(?<chat_id>/([a-f0-9]{32,40}|public))?$",
                ep_im_chat),

        entrypoint_rx("^/messages/im/waiting$",
                ep_im_waiting),

        // these two have the same output
        // FIXME: friends should probably not show local profiles
        entrypoint_rx("^/friends$",
                ep_friends), 
        entrypoint_rx("^/identities$",
                ep_friends), 

        // modify trust relationship with a given identity
        entrypoint_rx("^/friend/edit$",
                ep_friend_edit),

        // modify trust relationship with a given identity
        entrypoint_rx("^/friend/add$",
                ep_friend_add),
        entrypoint_rx("^/friends/add$",
                ep_friend_add),
        // {{{
        // lists available profiles and gpg identities
        entrypoint_rx("^/(my/)?profiles$",
                ep_profile_list),

        // displays the active profile
        entrypoint_rx("^/(my/)?profile/active$",
                ep_profile_active),

        // creates new gpg keys
        entrypoint_rx("^/(my/)?identity/create$",
                ep_pgp_identity_create),

        // creates new profiles
        entrypoint_rx("^/(my/)?profile/create$",
                ep_profile_create),

        // if we arent currently logged in, this will activate the 
        // RS profile in the URL using the password provided
        entrypoint_rx("^/(my/)?profile/activate$",
                ep_profile_activate),
        // }}}
        
        // only for browsing files
        // downloads must be initiated explicitly
        entrypoint_rx("^(?<_>/files/)(?<uid>[a-f0-9]{32,40})",
                ep_file_share_browse),

        // {{{
        // returns a list of all known forums and some info about them
        entrypoint_rx("^/forums$",
                ep_forum_index),

        // returns messages from the thread, either decending directly from the
        // root list or down from a specific sub-thread
        // or //
        // is used to post new messages on a forum
        entrypoint_rx("^/forum/[a-f0-9]{28,40}$",
                ep_forum_thread),

        // creates a new empty forum, only useful via POST
        entrypoint_rx("^/forum/create",
                ep_forum_create),
        // }}}
        //entrypoint_rx("^(?<_>/transfers/)(?<uid>[a-f0-9]{32,40})", ep_file_transfers),
        //entrypoint_rx("^/forum/)", ep_forum_view),
    };
}
#endif
