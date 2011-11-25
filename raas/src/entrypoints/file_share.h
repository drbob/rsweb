#ifndef RSWEB_ENTRYPOINT_FILE_SHARE_H
#define RSWEB_ENTRYPOINT_FILE_SHARE_H
#include <retroshare/rstypes.h>
#include "../entrypoint.h"

namespace rsweb {
void __output_DirDetails_FILE_data(evhttp_request* req, void* ref);
void __output_DirDetails_DIR_as_json(evhttp_request* req, void* ref, json_t* jroot);
};
#endif
