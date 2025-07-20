#include "jing_buffer.h"
#include <errno.h>
#include <string.h>

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