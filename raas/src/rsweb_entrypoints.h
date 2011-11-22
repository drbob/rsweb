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
    entrypoint_rx("^/static/.*", ep_static_files)
    entrypoint_rx("^/file_sharing/browse/.*", ep_file_share_browse)
};
};
#endif
