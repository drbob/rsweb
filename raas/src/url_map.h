#ifndef RSWEB_RAAS_ENTRY
#define RSWEB_RAAS_ENTRY

#include <vector>
#include <string>
#include <tuple>
#include <functional>
#include "entrypoint.h"
#include "entrypoints/enabled.h"

// This file defines the URLs that are enabled in the program
// Comparisons are made in the order they are defined in this file.
// Be careful to get the ordering right.

namespace rsweb {
std::vector<entrypoint_matcher_func_type> entrypoints = {
    entrypoint_rx("^/static/.*",
                  ep_static_files),
    
    entrypoint_rx("^/messages/global_chat$",
                  ep_global_chat),
   
    entrypoint_rx("^/messages/im", ep_im_chat),
    
    entrypoint_eq("/friends",
                  ep_friends), 
    
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

    //entrypoint_rx("^/forum/create", ep_forum_create)
    

    //entrypoint_rx("^/mystatus", ep_my_status),
    //entrypoint_rx("^(?<_>/transfers/)(?<uid>[a-f0-9]{32,40})", ep_file_transfers),
    //entrypoint_rx("^/forum/)", ep_forum_view),
};
};
#endif
