#ifndef MEM_H
#define MEM_H

#include "common.h"
#include <stddef.h>

EXPORT_SYMBOL void *jing_malloc(size_t size);

EXPORT_SYMBOL void jing_free(void *ptr);

EXPORT_SYMBOL void *jing_realloc(void *ptr, size_t size);

EXPORT_SYMBOL int jing_rp_initialize(void);

EXPORT_SYMBOL void jing_rp_thread_initialize(void);

EXPORT_SYMBOL void jing_rp_thread_finalize(void);

EXPORT_SYMBOL void jing_rp_finalize(void);

EXPORT_SYMBOL void *jing_rp_malloc(size_t size);

EXPORT_SYMBOL void jing_rp_free(void *ptr);

EXPORT_SYMBOL void *jing_rp_realloc(void *ptr, size_t size);

#endif
