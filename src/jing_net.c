#include "jing_net.h"
#include <errno.h>

int jing_connect_block_code() { return EINPROGRESS; }

int jing_send_block_code() { return EAGAIN; }
