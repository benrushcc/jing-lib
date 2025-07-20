#include "jing_deflate.h"
#include "jing_common.h"
#include "jing_malloc.h"
#include "libdeflate.h"
#include "mimalloc.h"
#include <errno.h>

int jing_deflate_success(void) { return LIBDEFLATE_SUCCESS; }

int jing_deflate_bad_data(void) { return LIBDEFLATE_BAD_DATA; }

int jing_deflate_short_output(void) { return LIBDEFLATE_SHORT_OUTPUT; }

int jing_deflate_insufficient_space(void) {
  return LIBDEFLATE_INSUFFICIENT_SPACE;
}

int jing_deflate_fastest_level(void) { return 1; }

int jing_deflate_default_level(void) { return 6; }

int jing_deflate_optimal_level(void) { return 12; }

struct libdeflate_compressor *jing_alloc_compressor(int compression_level) {
  return libdeflate_alloc_compressor(compression_level);
}

struct libdeflate_compressor *
jing_alloc_compressor_ex(int compression_level, malloc_func_t malloc_func,
                         free_func_t free_func) {
  struct libdeflate_options options = {.sizeof_options =
                                           sizeof(struct libdeflate_options),
                                       .malloc_func = malloc_func,
                                       .free_func = free_func};
  return libdeflate_alloc_compressor_ex(compression_level, &options);
}

void jing_free_compressor(struct libdeflate_compressor *compressor) {
  libdeflate_free_compressor(compressor);
}

struct libdeflate_decompressor *jing_alloc_decompressor(void) {
  return libdeflate_alloc_decompressor();
}

struct libdeflate_decompressor *
jing_alloc_decompressor_ex(malloc_func_t malloc_func, free_func_t free_func) {
  struct libdeflate_options options = {.sizeof_options =
                                           sizeof(struct libdeflate_options),
                                       .malloc_func = malloc_func,
                                       .free_func = free_func};
  return libdeflate_alloc_decompressor_ex(&options);
}

void jing_free_decompressor(struct libdeflate_decompressor *decompressor) {
  libdeflate_free_decompressor(decompressor);
}

size_t jing_deflate_compress(struct libdeflate_compressor *compressor,
                             const void *in, size_t in_nbytes, void *out,
                             size_t out_nbytes_avail) {
  return libdeflate_deflate_compress(compressor, in, in_nbytes, out,
                                     out_nbytes_avail);
}

size_t jing_deflate_compress_bound(struct libdeflate_compressor *compressor,
                                   size_t in_nbytes) {
  return libdeflate_deflate_compress_bound(compressor, in_nbytes);
}

size_t jing_zlib_compress(struct libdeflate_compressor *compressor,
                          const void *in, size_t in_nbytes, void *out,
                          size_t out_nbytes_avail) {
  return libdeflate_zlib_compress(compressor, in, in_nbytes, out,
                                  out_nbytes_avail);
}

size_t jing_zlib_compress_bound(struct libdeflate_compressor *compressor,
                                size_t in_nbytes) {
  return libdeflate_zlib_compress_bound(compressor, in_nbytes);
}

size_t jing_gzip_compress(struct libdeflate_compressor *compressor,
                          const void *in, size_t in_nbytes, void *out,
                          size_t out_nbytes_avail) {
  return libdeflate_gzip_compress(compressor, in, in_nbytes, out,
                                  out_nbytes_avail);
}

size_t jing_gzip_compress_bound(struct libdeflate_compressor *compressor,
                                size_t in_nbytes) {
  return libdeflate_gzip_compress_bound(compressor, in_nbytes);
}

int jing_deflate_decompress(struct libdeflate_decompressor *decompressor,
                            const void *in, size_t in_nbytes, void *out,
                            size_t out_nbytes_avail,
                            size_t *actual_out_nbytes_ret) {
  return libdeflate_deflate_decompress(decompressor, in, in_nbytes, out,
                                       out_nbytes_avail, actual_out_nbytes_ret);
}

int jing_zlib_decompress(struct libdeflate_decompressor *decompressor,
                         const void *in, size_t in_nbytes, void *out,
                         size_t out_nbytes_avail,
                         size_t *actual_out_nbytes_ret) {
  return libdeflate_zlib_decompress(decompressor, in, in_nbytes, out,
                                    out_nbytes_avail, actual_out_nbytes_ret);
}

int jing_gzip_decompress(struct libdeflate_decompressor *decompressor,
                         const void *in, size_t in_nbytes, void *out,
                         size_t out_nbytes_avail,
                         size_t *actual_out_nbytes_ret) {
  return libdeflate_gzip_decompress(decompressor, in, in_nbytes, out,
                                    out_nbytes_avail, actual_out_nbytes_ret);
}

typedef struct {
  size_t (*compress_bound)(struct libdeflate_compressor *, size_t);
  size_t (*compress)(struct libdeflate_compressor *, const void *, size_t,
                     void *, size_t);
} compressor_strategy_t;

static const compressor_strategy_t DEFLATE_COMPRESS_STRATEGY = {
    .compress_bound = jing_deflate_compress_bound,
    .compress = jing_deflate_compress};

static const compressor_strategy_t ZLIB_COMPRESS_STRATEGY = {
    .compress_bound = jing_zlib_compress_bound, .compress = jing_zlib_compress};

static const compressor_strategy_t GZIP_COMPRESS_STRATEGY = {
    .compress_bound = jing_gzip_compress_bound, .compress = jing_gzip_compress};

static inline void jing_compress_mi(const void *in, size_t len, int level,
                                    jing_result *result,
                                    const compressor_strategy_t *strategy) {
  int lv;
  if (likely(level >= jing_deflate_fastest_level() &&
             level <= jing_deflate_optimal_level())) {
    lv = level;
  } else {
    lv = jing_deflate_default_level();
  }
  struct libdeflate_compressor *cp =
      jing_alloc_compressor_ex(lv, mi_malloc, mi_free);
  if (unlikely(cp == NULL)) {
    jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
    return;
  }
  size_t out_len = strategy->compress_bound(cp, len);
  if (out_len == 0) {
    jing_free_compressor(cp);
    jing_err_result(result, EINVAL, JING_DEFAULT_FLAG);
    return;
  }
  void *out = mi_malloc(out_len);
  if (unlikely(out == NULL)) {
    jing_free_compressor(cp);
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t actual = strategy->compress(cp, in, len, out, out_len);
  if (actual == 0) {
    jing_free_compressor(cp);
    jing_err_result(result, EINVAL, JING_DEFAULT_FLAG);
    return;
  }
  jing_free_compressor(cp);
  jing_ptr_result(result, out, actual, JING_DEFAULT_FLAG);
}

void jing_deflate_compress_mi(const void *in, size_t len, int level,
                              jing_result *result) {
  jing_compress_mi(in, len, level, result, &DEFLATE_COMPRESS_STRATEGY);
}

void jing_zlib_compress_mi(const void *in, size_t len, int level,
                           jing_result *result) {
  jing_compress_mi(in, len, level, result, &ZLIB_COMPRESS_STRATEGY);
}

void jing_gzip_compress_mi(const void *in, size_t len, int level,
                           jing_result *result) {
  jing_compress_mi(in, len, level, result, &GZIP_COMPRESS_STRATEGY);
}

typedef struct {
  enum libdeflate_result (*decompress)(struct libdeflate_decompressor *,
                                       const void *, size_t, void *, size_t,
                                       size_t *);
} decompressor_strategy_t;

static const decompressor_strategy_t DEFLATE_DECOMPRESS_STRATEGY = {
    .decompress = libdeflate_deflate_decompress};

static const decompressor_strategy_t ZLIB_DECOMPRESS_STRATEGY = {
    .decompress = libdeflate_zlib_decompress};

static const decompressor_strategy_t GZIP_DECOMPRESS_STRATEGY = {
    .decompress = libdeflate_gzip_decompress};

static inline void jing_decompress_mi(const void *in, size_t len,
                                      size_t initial_size, size_t limit,
                                      jing_result *result,
                                      const decompressor_strategy_t *strategy) {
  if (unlikely(initial_size >= limit)) {
    jing_err_result(result, EINVAL, JING_DEFAULT_FLAG);
    return;
  }
  struct libdeflate_decompressor *dcp =
      jing_alloc_decompressor_ex(mi_malloc, mi_free);
  if (unlikely(dcp == NULL)) {
    jing_err_result(result, EINVAL, JING_DEFAULT_FLAG);
    return;
  }
  size_t out_len = initial_size;
  void *out = mi_malloc(out_len);
  if (out == NULL) {
    jing_free_decompressor(dcp);
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t actual;
  for (;;) {
    switch (strategy->decompress(dcp, in, len, out, out_len, &actual)) {
    case LIBDEFLATE_SUCCESS: {
      jing_free_decompressor(dcp);
      jing_ptr_result(result, out, actual, JING_DEFAULT_FLAG);
      return;
    }
    case LIBDEFLATE_INSUFFICIENT_SPACE: {
      if (out_len > limit / 2) {
        jing_err_result(result, EOVERFLOW, JING_DEFAULT_FLAG);
        return;
      }
      out_len = out_len * 2;
      void *new_out = mi_realloc(out, out_len);
      if (new_out == NULL) {
        mi_free(out);
        jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
        return;
      }
      out = new_out;
      break;
    }
    default: {
      jing_free_decompressor(dcp);
      jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
      return;
    }
    }
  }
}

void jing_deflate_decompress_mi(const void *in, size_t len,
                                size_t original_size, size_t limit,
                                jing_result *result) {
  jing_decompress_mi(in, len, original_size, limit, result,
                     &DEFLATE_DECOMPRESS_STRATEGY);
}

void jing_zlib_decompress_mi(const void *in, size_t len, size_t original_size,
                             size_t limit, jing_result *result) {
  jing_decompress_mi(in, len, original_size, limit, result,
                     &ZLIB_DECOMPRESS_STRATEGY);
}

void jing_gzip_decompress_mi(const void *in, size_t len, size_t original_size,
                             size_t limit, jing_result *result) {
  jing_decompress_mi(in, len, original_size, limit, result,
                     &GZIP_DECOMPRESS_STRATEGY);
}