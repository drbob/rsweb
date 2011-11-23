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
    entrypoint_eq("/friends", ep_friends), 
    entrypoint_rx("^/messages/global_chat$", ep_global_chat),
    entrypoint_rx("^/static/.*", ep_static_files),
    entrypoint_rx("^(?<_>/file_sharing/)(?<uid>[a-f0-9]{32,40})", ep_file_share_browse),

//    entrypoint_rx("^/forums)", ep_forum_index),
  //  entrypoint_rx("^/forum/)", ep_forum_view),
    //entrypoint_rx("^/presence/mine", ep_presence_mine),
};
};
#endif
