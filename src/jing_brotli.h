#ifndef JING_BROTLI_H
#define JING_BROTLI_H

#include "brotli/decode.h"
#include "jing_common.h"

typedef struct {
  malloc_func_t malloc_func;
  free_func_t free_func;
} allocator;

JING_EXPORT_SYMBOL int jing_brotli_min_window_bits(void);

JING_EXPORT_SYMBOL int jing_brotli_max_window_bits(void);

JING_EXPORT_SYMBOL int jing_brotli_default_window_bits(void);

JING_EXPORT_SYMBOL int jing_brotli_min_quality(void);

JING_EXPORT_SYMBOL int jing_brotli_max_quality(void);

JING_EXPORT_SYMBOL int jing_brotli_default_quality(void);

JING_EXPORT_SYMBOL int jing_brotli_mode_generic(void);

JING_EXPORT_SYMBOL int jing_brotli_mode_text(void);

JING_EXPORT_SYMBOL int jing_brotli_mode_font(void);

JING_EXPORT_SYMBOL int jing_brotli_decoder_result_error(void);

JING_EXPORT_SYMBOL int jing_brotli_decoder_result_success(void);

JING_EXPORT_SYMBOL int jing_brotli_decoder_result_needs_more_input(void);

JING_EXPORT_SYMBOL int jing_brotli_decoder_result_needs_more_output(void);

JING_EXPORT_SYMBOL int jing_brotli_true(void);

JING_EXPORT_SYMBOL int jing_brotli_false(void);

JING_EXPORT_SYMBOL size_t
jing_brotli_encoder_max_compressed_size(size_t input_size);

JING_EXPORT_SYMBOL int jing_brotli_encoder_compress(int quality, int lgwin,
                                                    int mode, size_t input_size,
                                                    const uint8_t *input_buffer,
                                                    size_t *encoded_size,
                                                    uint8_t *encoded_buffer);

JING_EXPORT_SYMBOL void jing_brotli_compress_mi(void *in, size_t len, int level,
                                                int window_bits, int mode,
                                                jing_result *result);

JING_EXPORT_SYMBOL BrotliDecoderState *
jing_brotli_decoder_create_instance(void);

JING_EXPORT_SYMBOL BrotliDecoderState *
jing_brotli_decoder_create_instance_ex(malloc_func_t malloc_func,
                                       free_func_t free_func);

JING_EXPORT_SYMBOL void
jing_brotli_decoder_destroy_instance(BrotliDecoderState *state);

JING_EXPORT_SYMBOL int jing_brotli_decoder_decompress_stream(
    BrotliDecoderState *state, size_t *available_in, const uint8_t **next_in,
    size_t *available_out, uint8_t **next_out, size_t *total_out);

JING_EXPORT_SYMBOL void jing_brotli_decompress_mi(const void *in, size_t len,
                                                  size_t initial_size,
                                                  size_t limit,
                                                  jing_result *result);

JING_EXPORT_SYMBOL void jing_brotli_decompress_stream_mi(const void *in,
                                                         size_t len,
                                                         size_t initial_size,
                                                         size_t limit,
                                                         jing_result *result);

#endif