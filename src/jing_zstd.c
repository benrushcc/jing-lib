#include "jing_zstd.h"
#include "jing_common.h"
#include "mimalloc.h"
#include "zstd.h"
#include <errno.h>

static_assert(sizeof(unsigned long long) == 8,
              "unsigned long long size mismatch");
static_assert(sizeof(unsigned) == 4, "unsigned size mismatch");

int jing_zstd_flag() { return JING_ZSTD_FLAG; }

unsigned jing_zstd_is_error(size_t result) { return ZSTD_isError(result); }

const char *jing_zstd_get_error_name(size_t result) {
  return ZSTD_getErrorName(result);
}

int jing_zstd_min_compression_level() { return ZSTD_minCLevel(); }

int jing_zstd_max_compression_level() { return ZSTD_maxCLevel(); }

int jing_zstd_default_compression_level() { return ZSTD_defaultCLevel(); }

size_t jing_zstd_compress(void *dst, size_t dst_capacity, const void *src,
                          size_t src_size, int compression_level) {
  return ZSTD_compress(dst, dst_capacity, src, src_size, compression_level);
}

size_t jing_zstd_compress_bound(size_t src_size) {
  return ZSTD_compressBound(src_size);
}

size_t jing_zstd_decompress(void *dst, size_t dst_capacity, const void *src,
                            size_t compressed_size) {
  return ZSTD_decompress(dst, dst_capacity, src, compressed_size);
}

unsigned long long jing_zstd_content_unknown() {
  return ZSTD_CONTENTSIZE_UNKNOWN;
}

unsigned long long jing_zstd_contentsize_error() {
  return ZSTD_CONTENTSIZE_ERROR;
}

unsigned long long jing_zstd_get_frame_content_size(const void *src,
                                                    size_t src_size) {
  return ZSTD_getFrameContentSize(src, src_size);
}

ZSTD_DStream *jing_zstd_create_decompress_stream() {
  return ZSTD_createDStream();
}

size_t jing_zstd_free_decompress_stream(ZSTD_DStream *zds) {
  return ZSTD_freeDStream(zds);
}

size_t jing_zstd_init_decompress_stream(ZSTD_DStream *zds) {
  return ZSTD_initDStream(zds);
}

size_t jing_zstd_decompress_stream(ZSTD_DStream *zds, ZSTD_outBuffer *output,
                                   ZSTD_inBuffer *input) {
  return ZSTD_decompressStream(zds, output, input);
}

size_t jing_zstd_stream_out_size() { return ZSTD_DStreamOutSize(); }

void jing_zstd_compress_mi(void *in, size_t len, int level, size_t limit,
                           jing_result *result) {
  size_t bound = ZSTD_compressBound(len);
  if (ZSTD_isError(bound) != 0) {
    jing_err_result(result, ZSTD_getErrorCode(bound), JING_ZSTD_FLAG);
    return;
  }
  if (bound > limit) {
    jing_err_result(result, ZSTD_error_dstSize_tooSmall, JING_ZSTD_FLAG);
    return;
  }
  void *out = mi_malloc(bound);
  if (out == NULL) {
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t out_len = ZSTD_compress(out, bound, in, len, level);
  if (ZSTD_isError(len) != 0) {
    mi_free(out);
    jing_err_result(result, ZSTD_getErrorCode(len), JING_ZSTD_FLAG);
    return;
  }
  jing_ptr_result(result, out, out_len, JING_DEFAULT_FLAG);
}

void jing_zstd_decompress_mi(void *in, size_t len, size_t limit,
                             jing_result *result) {
  unsigned long long out_len = ZSTD_getFrameContentSize(in, len);
  if (out_len == ZSTD_CONTENTSIZE_ERROR) {
    jing_err_result(result, ZSTD_error_parameter_unsupported, JING_ZSTD_FLAG);
    return;
  } else if (out_len == ZSTD_CONTENTSIZE_UNKNOWN) {
    size_t buffer_size = ZSTD_DStreamOutSize();
    if (ZSTD_isError(buffer_size)) {
      jing_err_result(result, ZSTD_getErrorCode(buffer_size), JING_ZSTD_FLAG);
      return;
    }
    jing_zstd_decompress_stream_mi(in, len, buffer_size, limit, result);
    return;
  } else {
    void *out = mi_malloc(out_len);
    if (out == NULL) {
      jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
      return;
    }
    size_t actual_len = ZSTD_decompress(out, out_len, in, len);
    if (ZSTD_isError(actual_len) != 0) {
      mi_free(out);
      jing_err_result(result, ZSTD_getErrorCode(actual_len), JING_ZSTD_FLAG);
      return;
    }
    jing_ptr_result(result, out, actual_len, JING_DEFAULT_FLAG);
  }
}

void jing_zstd_decompress_stream_mi(void *in, size_t len, size_t initial_size,
                                    size_t limit, jing_result *result) {
  jing_buffer b;
  int r = jing_buffer_init(&b, initial_size, mi_malloc, mi_realloc, mi_free);
  if (r > 0) {
    jing_err_result(result, r, JING_DEFAULT_FLAG);
    return;
  }
  void *buffer = mi_malloc(initial_size);
  if (buffer == NULL) {
    jing_buffer_free(&b);
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  ZSTD_DStream *stream = ZSTD_createDStream();
  if (stream == NULL) {
    mi_free(buffer);
    jing_buffer_free(&b);
    jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
    return;
  }
  ZSTD_initDStream(stream);
  ZSTD_inBuffer in_buffer = {.src = in, .size = len, .pos = 0};
  ZSTD_outBuffer out_buffer = {.dst = buffer, .size = initial_size, .pos = 0};
  for (;;) {
    size_t ret = ZSTD_decompressStream(stream, &out_buffer, &in_buffer);
    if (ret == 0) {
      ZSTD_freeDStream(stream);
      r = jing_buffer_append(&b, buffer, out_buffer.pos, limit);
      mi_free(buffer);
      if (r > 0) {
        jing_buffer_free(&b);
        jing_err_result(result, r, JING_DEFAULT_FLAG);
        return;
      }
      jing_ptr_result(result, b.ptr, b.used, JING_DEFAULT_FLAG);
      return;
    } else if (ZSTD_isError(ret)) {
      ZSTD_freeDStream(stream);
      mi_free(buffer);
      jing_buffer_free(&b);
      jing_err_result(result, ZSTD_getErrorCode(ret), JING_ZSTD_FLAG);
      return;
    } else {
      r = jing_buffer_append(&b, buffer, out_buffer.pos, limit);
      if (r > 0) {
        ZSTD_freeDStream(stream);
        mi_free(buffer);
        jing_buffer_free(&b);
        jing_err_result(result, r, JING_DEFAULT_FLAG);
        return;
      }
      out_buffer.pos = 0;
    }
  }
}