#ifndef JING_MI_MALLOC_H
#define JING_MI_MALLOC_H

#include "jing_malloc.h"

JING_EXPORT_SYMBOL malloc_func_t jing_mi_malloc_funtion(void);

JING_EXPORT_SYMBOL realloc_func_t jing_mi_realloc_function(void);

JING_EXPORT_SYMBOL free_func_t jing_mi_free_function(void);

JING_EXPORT_SYMBOL void *jing_mi_malloc(size_t size);

JING_EXPORT_SYMBOL void jing_mi_free(void *ptr);

JING_EXPORT_SYMBOL void *jing_mi_realloc(void *ptr, size_t size);

JING_EXPORT_SYMBOL void *jing_mi_aligned_alloc(size_t alignment, size_t size);

JING_EXPORT_SYMBOL void jing_mi_aligned_free(void *ptr);

#endif