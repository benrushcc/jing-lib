#include "jing_rpmalloc.h"
#include "jing_common.h"
#include "rpmalloc.h"

int jing_rp_initialize(void) { return rpmalloc_initialize(); }

void jing_rp_finalize(void) { rpmalloc_finalize(); }

void jing_rp_thread_initialize(void) { rpmalloc_thread_initialize(); }

void jing_rp_thread_finalize(void) { rpmalloc_thread_finalize(1); }

void *jing_rp_malloc(size_t size) { return rpmalloc(size); }

void jing_rp_free(void *ptr) { rpfree(ptr); }

void *jing_rp_realloc(void *ptr, size_t size) { return rprealloc(ptr, size); }

void *jing_rp_aligned_alloc(size_t alignment, size_t size) {
  return rpaligned_alloc(alignment, size);
}