#include "jing_common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void jing_err_result(jing_result *r, int err, int flag) {
  r->err = err;
  r->flag = flag;
  r->len = 0;
  r->data.ptr_val = NULL;
}

void jing_byte_result(jing_result *r, int8_t value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(int8_t);
  r->data.byte_val = value;
}

void jing_short_result(jing_result *r, int16_t value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(int16_t);
  r->data.short_val = value;
}

void jing_int_result(jing_result *r, int32_t value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(int32_t);
  r->data.int_val = value;
}

void jing_long_result(jing_result *r, int64_t value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(int64_t);
  r->data.long_val = value;
}

void jing_float_result(jing_result *r, float value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(float);
  r->data.float_val = value;
}

void jing_double_result(jing_result *r, double value, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = sizeof(double);
  r->data.double_val = value;
}

void jing_ptr_result(jing_result *r, void *value, size_t len, int flag) {
  r->err = JING_NO_ERR;
  r->flag = flag;
  r->len = len;
  r->data.ptr_val = value;
}

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

int jing_buffer_init(jing_buffer *buffer, size_t init_size, malloc_func_t mc,
                     realloc_func_t rc, free_func_t fc) {
  void *b = mc(init_size);
  if (b == NULL) {
    return ENOMEM;
  }
  memset(buffer, 0, sizeof(jing_buffer));
  buffer->ptr = b;
  buffer->len = init_size;
  buffer->malloc_func = mc;
  buffer->realloc_func = rc;
  buffer->free_func = fc;
  return JING_NO_ERR;
}

int jing_buffer_append(jing_buffer *buffer, void *in, size_t size,
                       size_t limit) {
  if (size == 0) {
    return JING_NO_ERR;
  }
  void *ptr = buffer->ptr;
  size_t used = buffer->used;
  size_t len = buffer->len;
  size_t new_used = used + size;
  if (used > SIZE_MAX - len || new_used > limit) {
    return EOVERFLOW;
  }
  if (new_used > len) {
    size_t new_len = len;
    while (new_len < new_used) {
      if (new_len > SIZE_MAX / 2) {
        return EOVERFLOW;
      }
      new_len *= 2;
    }
    void *new_ptr = buffer->realloc_func(ptr, new_len);
    if (new_ptr == NULL) {
      return ENOMEM;
    }
    memcpy((char *)new_ptr + used, in, size);
    buffer->ptr = new_ptr;
    buffer->len = new_len;
    buffer->used = new_used;
  } else {
    memcpy((char *)ptr + used, in, size);
    buffer->used = new_used;
  }
  return JING_NO_ERR;
}

void jing_buffer_free(jing_buffer *buffer) {
  void *ptr = buffer->ptr;
  if (ptr != NULL) {
    buffer->free_func(ptr);
  }
}

#ifdef JING_ENABLE_TESTS

#include <errno.h>
#include <stdarg.h>

int jing_test_int(void) { return 0; }

int jing_test_errno(int err) {
  errno = err;
  return err;
}

int jing_test_sum_vargs(int count, ...) {
  va_list args;
  int sum = 0;
  int i;
  va_start(args, count);
  for (i = 0; i < count; i++) {
    sum += va_arg(args, int);
  }
  va_end(args);
  return sum;
}

#endif