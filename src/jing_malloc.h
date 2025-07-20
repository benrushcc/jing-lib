#ifndef JING_MALLOC_H
#define JING_MALLOC_H

#include "jing_common.h"

typedef void *(*malloc_func_t)(size_t size);

typedef void *(*realloc_func_t)(void *ptr, size_t size);

typedef void (*free_func_t)(void *ptr);

JING_EXPORT_SYMBOL malloc_func_t jing_malloc_funtion(void);

JING_EXPORT_SYMBOL realloc_func_t jing_realloc_function(void);

JING_EXPORT_SYMBOL free_func_t jing_free_function(void);

JING_EXPORT_SYMBOL void *jing_malloc(size_t size);

JING_EXPORT_SYMBOL void jing_free(void *ptr);

JING_EXPORT_SYMBOL void *jing_realloc(void *ptr, size_t size);

JING_EXPORT_SYMBOL void *jing_aligned_alloc(size_t alignment, size_t size);

JING_EXPORT_SYMBOL void jing_aligned_free(void *ptr);

#endif