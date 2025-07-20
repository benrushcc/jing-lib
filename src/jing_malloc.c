#include "jing_malloc.h"
#include "jing_common.h"
#include <stdlib.h>

malloc_func_t jing_malloc_funtion(void) { return malloc; }

realloc_func_t jing_realloc_function(void) { return realloc; }

free_func_t jing_free_function(void) { return free; }

void *jing_malloc(size_t size) { return malloc(size); }

void jing_free(void *ptr) { free(ptr); }

void *jing_realloc(void *ptr, size_t size) { return realloc(ptr, size); }

void *jing_aligned_alloc(size_t alignment, size_t size) {
#ifdef OS_WINDOWS
  return _aligned_malloc(size, alignment);
#else
  return aligned_alloc(alignment, size);
#endif
}

void jing_aligned_free(void *ptr) {
#ifdef OS_WINDOWS
  _aligned_free(ptr);
#else
  free(ptr);
#endif
}
