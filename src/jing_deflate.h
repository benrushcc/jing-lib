#ifndef JING_DEFLATE_H
#define JING_DEFLATE_H

#include "jing_common.h"
#include "jing_malloc.h"

JING_EXPORT_SYMBOL int jing_deflate_success(void);

JING_EXPORT_SYMBOL int jing_deflate_bad_data(void);

JING_EXPORT_SYMBOL int jing_deflate_short_output(void);

JING_EXPORT_SYMBOL int jing_deflate_insufficient_space(void);

JING_EXPORT_SYMBOL int jing_deflate_fastest_level(void);

JING_EXPORT_SYMBOL int jing_deflate_default_level(void);

JING_EXPORT_SYMBOL int jing_deflate_optimal_level(void);

JING_EXPORT_SYMBOL struct libdeflate_compressor *
jing_alloc_compressor(int compression_level);

JING_EXPORT_SYMBOL struct libdeflate_compressor *
jing_alloc_compressor_ex(int compression_level, malloc_func_t malloc_func,
                         free_func_t free_func);

JING_EXPORT_SYMBOL void
jing_free_compressor(struct libdeflate_compressor *compressor);

JING_EXPORT_SYMBOL struct libdeflate_decompressor *
jing_alloc_decompressor(void);

JING_EXPORT_SYMBOL struct libdeflate_decompressor *
jing_alloc_decompressor_ex(malloc_func_t malloc_func, free_func_t free_func);

JING_EXPORT_SYMBOL void
jing_free_decompressor(struct libdeflate_decompressor *decompressor);

JING_EXPORT_SYMBOL size_t
jing_deflate_compress(struct libdeflate_compressor *compressor, const void *in,
                      size_t in_nbytes, void *out, size_t out_nbytes_avail);

JING_EXPORT_SYMBOL size_t jing_deflate_compress_bound(
    struct libdeflate_compressor *compressor, size_t in_nbytes);

JING_EXPORT_SYMBOL size_t
jing_zlib_compress(struct libdeflate_compressor *compressor, const void *in,
                   size_t in_nbytes, void *out, size_t out_nbytes_avail);

JING_EXPORT_SYMBOL size_t jing_zlib_compress_bound(
    struct libdeflate_compressor *compressor, size_t in_nbytes);

JING_EXPORT_SYMBOL size_t
jing_gzip_compress(struct libdeflate_compressor *compressor, const void *in,
                   size_t in_nbytes, void *out, size_t out_nbytes_avail);

JING_EXPORT_SYMBOL size_t jing_gzip_compress_bound(
    struct libdeflate_compressor *compressor, size_t in_nbytes);

JING_EXPORT_SYMBOL int
jing_deflate_decompress(struct libdeflate_decompressor *decompressor,
                        const void *in, size_t in_nbytes, void *out,
                        size_t out_nbytes_avail, size_t *actual_out_nbytes_ret);

JING_EXPORT_SYMBOL int
jing_zlib_decompress(struct libdeflate_decompressor *decompressor,
                     const void *in, size_t in_nbytes, void *out,
                     size_t out_nbytes_avail, size_t *actual_out_nbytes_ret);

JING_EXPORT_SYMBOL int
jing_gzip_decompress(struct libdeflate_decompressor *decompressor,
                     const void *in, size_t in_nbytes, void *out,
                     size_t out_nbytes_avail, size_t *actual_out_nbytes_ret);

JING_EXPORT_SYMBOL void jing_deflate_compress_mi(const void *in, size_t len,
                                                 int level,
                                                 jing_result *result);

JING_EXPORT_SYMBOL void jing_zlib_compress_mi(const void *in, size_t len,
                                              int level, jing_result *result);

JING_EXPORT_SYMBOL void jing_gzip_compress_mi(const void *in, size_t len,
                                              int level, jing_result *result);

JING_EXPORT_SYMBOL void jing_deflate_decompress_mi(const void *in, size_t len,
                                                   size_t original_size,
                                                   size_t limit,
                                                   jing_result *result);

JING_EXPORT_SYMBOL void jing_zlib_decompress_mi(const void *in, size_t len,
                                                size_t original_size,
                                                size_t limit,
                                                jing_result *result);

JING_EXPORT_SYMBOL void jing_gzip_decompress_mi(const void *in, size_t len,
                                                size_t original_size,
                                                size_t limit,
                                                jing_result *result);

#endif