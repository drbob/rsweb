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

        entrypoint_rx("^/friends$",
                ep_friends), 
        // FIXME: should be like /friends except also add other idents we know
        // about such as our own local ones
        entrypoint_rx("^/identities$",
                ep_friends), 

        // lists available profiles
        entrypoint_rx("^/profiles$",
                ep_profile_list),

        // if we arent currently logged in, this will activate the 
        // RS profile in the URL using the password provided
        entrypoint_rx("^/profile/activate$",
                ep_profile_activate),
        entrypoint_rx("^(?<_>/file_sharing/)(?<uid>[a-f0-9]{32,40})",
                ep_file_share_browse),

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
                ep_forum_create)

            //entrypoint_rx("^(?<_>/transfers/)(?<uid>[a-f0-9]{32,40})", ep_file_transfers),
            //entrypoint_rx("^/forum/)", ep_forum_view),
    };
}
#endif
