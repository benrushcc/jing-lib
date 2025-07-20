#ifndef JING_BUFFER_H
#define JING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "jing_common.h"
#include "jing_malloc.h"

typedef struct {
  void *ptr;
  size_t used;
  size_t len;
  malloc_func_t malloc_func;
  realloc_func_t realloc_func;
  free_func_t free_func;
} jing_buffer;

JING_HIDDEN_SYMBOL int jing_buffer_init(jing_buffer *buffer, size_t init_size,
                                        malloc_func_t mc, realloc_func_t rc,
                                        free_func_t fc);

JING_HIDDEN_SYMBOL int jing_buffer_append(jing_buffer *buffer, void *in,
                                          size_t len, size_t limit);

JING_HIDDEN_SYMBOL void jing_buffer_free(jing_buffer *buffer);

#ifdef __cplusplus
}
#endif

#endif