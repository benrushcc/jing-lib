#ifndef JING_ZSTD_H
#define JING_ZSTD_H

#define JING_ZSTD_FLAG 60937

#include "jing_buffer.h"
#include "jing_common.h"
#include "zstd.h"


JING_EXPORT_SYMBOL int jing_zstd_flag(void);

JING_EXPORT_SYMBOL unsigned jing_zstd_is_error(size_t result);

JING_EXPORT_SYMBOL const char *jing_zstd_get_error_name(size_t result);

JING_EXPORT_SYMBOL int jing_zstd_min_compression_level(void);

JING_EXPORT_SYMBOL int jing_zstd_max_compression_level(void);

JING_EXPORT_SYMBOL int jing_zstd_default_compression_level(void);

JING_EXPORT_SYMBOL size_t jing_zstd_compress(void *dst, size_t dst_capacity,
                                             const void *src, size_t src_size,
                                             int compression_level);

JING_EXPORT_SYMBOL size_t jing_zstd_compress_bound(size_t src_size);

JING_EXPORT_SYMBOL size_t jing_zstd_decompress(void *dst, size_t dst_capacity,
                                               const void *src,
                                               size_t compressed_size);

JING_EXPORT_SYMBOL unsigned long long jing_zstd_content_unknown(void);

JING_EXPORT_SYMBOL unsigned long long jing_zstd_contentsize_error(void);

JING_EXPORT_SYMBOL unsigned long long
jing_zstd_get_frame_content_size(const void *src, size_t src_size);

JING_EXPORT_SYMBOL ZSTD_DStream *jing_zstd_create_decompress_stream(void);

JING_EXPORT_SYMBOL size_t jing_zstd_free_decompress_stream(ZSTD_DStream *zds);

JING_EXPORT_SYMBOL size_t jing_zstd_init_decompress_stream(ZSTD_DStream *zds);

JING_EXPORT_SYMBOL size_t jing_zstd_decompress_stream(ZSTD_DStream *zds,
                                                      ZSTD_outBuffer *output,
                                                      ZSTD_inBuffer *input);

JING_EXPORT_SYMBOL size_t jing_zstd_stream_out_size(void);

JING_EXPORT_SYMBOL void jing_zstd_compress_mi(void *in, size_t len, int level,
                                              size_t limit,
                                              jing_result *result);

JING_EXPORT_SYMBOL void jing_zstd_decompress_mi(void *in, size_t len,
                                                size_t limit,
                                                jing_result *result);

JING_EXPORT_SYMBOL void jing_zstd_decompress_stream_mi(void *in, size_t len,
                                                       size_t initial_size,
                                                       size_t limit,
                                                       jing_result *result);

#endif