#include "jing_mimalloc.h"
#include "jing_common.h"
#include "jing_malloc.h"
#include "mimalloc.h"


malloc_func_t jing_mi_malloc_funtion(void) { return mi_malloc; }

realloc_func_t jing_mi_realloc_function(void) { return mi_realloc; }

free_func_t jing_mi_free_function(void) { return mi_free; }

void *jing_mi_malloc(size_t size) { return mi_malloc(size); }

void jing_mi_free(void *ptr) { mi_free(ptr); }

void *jing_mi_realloc(void *ptr, size_t size) { return mi_realloc(ptr, size); }

void *jing_mi_aligned_alloc(size_t alignment, size_t size) {
  return mi_aligned_alloc(alignment, size);
}

void jing_mi_aligned_free(void *ptr) { mi_free(ptr); }