#include "jing_quic.h"
#include "jing_common.h"
#include "lsquic.h"

int jing_quic_major_version() { return LSQUIC_MAJOR_VERSION; }

int jing_quic_minor_version() { return LSQUIC_MINOR_VERSION; }

int jing_quic_patch_version() { return LSQUIC_PATCH_VERSION; }
