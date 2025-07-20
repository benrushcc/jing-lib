#ifndef JING_RP_MALLOC_H
#define JING_RP_MALLOC_H

#include "jing_malloc.h"

JING_EXPORT_SYMBOL malloc_func_t jing_rp_malloc_funtion(void);

JING_EXPORT_SYMBOL realloc_func_t jing_rp_realloc_function(void);

JING_EXPORT_SYMBOL free_func_t jing_rp_free_function(void);

JING_EXPORT_SYMBOL int jing_rp_initialize(void);

JING_EXPORT_SYMBOL void jing_rp_finalize(void);

JING_EXPORT_SYMBOL void jing_rp_thread_initialize(void);

JING_EXPORT_SYMBOL void jing_rp_thread_finalize(void);

JING_EXPORT_SYMBOL void *jing_rp_malloc(size_t size);

JING_EXPORT_SYMBOL void jing_rp_free(void *ptr);

JING_EXPORT_SYMBOL void *jing_rp_realloc(void *ptr, size_t size);

JING_EXPORT_SYMBOL void *jing_rp_aligned_alloc(size_t alignment, size_t size);

#endif