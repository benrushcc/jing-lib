#include "jing_brotli.h"
#include "brotli/decode.h"
#include "brotli/encode.h"
#include "jing_common.h"
#include "mimalloc.h"
#include <errno.h>

int jing_brotli_min_window_bits(void) { return BROTLI_MIN_WINDOW_BITS; }

int jing_brotli_max_window_bits(void) { return BROTLI_MAX_WINDOW_BITS; }

int jing_brotli_default_window_bits(void) { return BROTLI_DEFAULT_WINDOW; }

int jing_brotli_min_quality(void) { return BROTLI_MIN_QUALITY; }

int jing_brotli_max_quality(void) { return BROTLI_MAX_QUALITY; }

int jing_brotli_default_quality(void) { return BROTLI_DEFAULT_QUALITY; }

int jing_brotli_mode_generic(void) { return BROTLI_MODE_GENERIC; }

int jing_brotli_mode_text(void) { return BROTLI_MODE_TEXT; }

int jing_brotli_mode_font(void) { return BROTLI_MODE_FONT; }

int jing_brotli_decoder_result_error(void) {
  return BROTLI_DECODER_RESULT_ERROR;
}

int jing_brotli_decoder_result_success(void) {
  return BROTLI_DECODER_RESULT_SUCCESS;
}

int jing_brotli_decoder_result_needs_more_input(void) {
  return BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
}

int jing_brotli_decoder_result_needs_more_output(void) {
  return BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT;
}

int jing_brotli_true(void) { return BROTLI_TRUE; }

int jing_brotli_false(void) { return BROTLI_FALSE; }

size_t jing_brotli_encoder_max_compressed_size(size_t input_size) {
  return BrotliEncoderMaxCompressedSize(input_size);
}

int jing_brotli_encoder_compress(int quality, int lgwin, int mode,
                                 size_t input_size, const uint8_t *input_buffer,
                                 size_t *encoded_size,
                                 uint8_t *encoded_buffer) {
  return BrotliEncoderCompress(quality, lgwin, mode, input_size, input_buffer,
                               encoded_size, encoded_buffer);
}

static void *malloc_adapter(void *opaque, size_t size) {
  return ((allocator *)opaque)->malloc_func(size);
}

static void free_adapter(void *opaque, void *ptr) {
  ((allocator *)opaque)->free_func(ptr);
}

BrotliDecoderState *jing_brotli_decoder_create_instance(void) {
  return BrotliDecoderCreateInstance(NULL, NULL, NULL);
}

BrotliDecoderState *
jing_brotli_decoder_create_instance_ex(malloc_func_t malloc_func,
                                       free_func_t free_func) {
  allocator alloc = {.malloc_func = malloc_func, .free_func = free_func};
  return BrotliDecoderCreateInstance(malloc_adapter, free_adapter, &alloc);
}

void jing_brotli_decoder_destroy_instance(BrotliDecoderState *state) {
  BrotliDecoderDestroyInstance(state);
}

int jing_brotli_decoder_decompress_stream(
    BrotliDecoderState *state, size_t *available_in, const uint8_t **next_in,
    size_t *available_out, uint8_t **next_out, size_t *total_out) {
  return BrotliDecoderDecompressStream(state, available_in, next_in,
                                       available_out, next_out, total_out);
}

void jing_brotli_compress_mi(void *in, size_t len, int level, int window_bits,
                             int mode, jing_result *result) {
  size_t out_len = BrotliEncoderMaxCompressedSize(len);
  if (out_len == 0) {
    jing_err_result(result, EINVAL, JING_DEFAULT_FLAG);
    return;
  }
  void *out = mi_malloc(out_len);
  if (out == NULL) {
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t actual;
  int r =
      BrotliEncoderCompress(level, window_bits, mode, len, in, &actual, out);
  if (r == BROTLI_TRUE) {
    jing_ptr_result(result, out, actual, JING_DEFAULT_FLAG);
  } else {
    mi_free(out);
    jing_err_result(result, EIO, JING_DEFAULT_FLAG);
  }
}

void jing_brotli_decompress_mi(const void *in, size_t len, size_t initial_size,
                               size_t limit, jing_result *result) {
  BrotliDecoderState *state =
      jing_brotli_decoder_create_instance_ex(mi_malloc, mi_free);
  if (state == NULL) {
    jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
    return;
  }
  void *out = mi_malloc(initial_size);
  if (out == NULL) {
    jing_brotli_decoder_destroy_instance(state);
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t available_in = len;
  size_t total_out = initial_size;
  size_t available_out = initial_size;
  uint8_t *out_ptr = out;
  for (;;) {
    switch (BrotliDecoderDecompressStream(state, &available_in,
                                          (const uint8_t **)&in, &available_out,
                                          &out_ptr, NULL)) {
    case BROTLI_DECODER_RESULT_SUCCESS: {
      jing_brotli_decoder_destroy_instance(state);
      jing_ptr_result(result, out, total_out - available_out,
                      JING_DEFAULT_FLAG);
      return;
    }
    case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
      if (total_out > limit / 2) {
        jing_brotli_decoder_destroy_instance(state);
        mi_free(out);
        jing_err_result(result, EOVERFLOW, JING_DEFAULT_FLAG);
        return;
      }
      total_out = total_out * 2;
      out = mi_realloc(out, total_out);
      if (out == NULL) {
        jing_brotli_decoder_destroy_instance(state);
        jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
        return;
      }
      size_t offset = out_ptr - (uint8_t *)out;
      out_ptr = (uint8_t *)out + offset;
      available_out = total_out - offset;
      break;
    }
    default: {
      jing_brotli_decoder_destroy_instance(state);
      mi_free(out);
      jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
      return;
    }
    }
  }
}

void jing_brotli_decompress_stream_mi(const void *in, size_t len,
                                      size_t initial_size, size_t limit,
                                      jing_result *result) {
  BrotliDecoderState *state =
      jing_brotli_decoder_create_instance_ex(mi_malloc, mi_free);
  if (state == NULL) {
    jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
    return;
  }
  jing_buffer b;
  int r = jing_buffer_init(&b, initial_size, mi_malloc, mi_realloc, mi_free);
  if (r > 0) {
    jing_brotli_decoder_destroy_instance(state);
    jing_err_result(result, r, JING_DEFAULT_FLAG);
    return;
  }
  void *out = mi_malloc(initial_size);
  if (out == NULL) {
    jing_brotli_decoder_destroy_instance(state);
    jing_buffer_free(&b);
    jing_err_result(result, ENOMEM, JING_DEFAULT_FLAG);
    return;
  }
  size_t available_in = len;
  size_t available_out = initial_size;
  uint8_t *out_ptr = out;
  for (;;) {
    switch (BrotliDecoderDecompressStream(state, &available_in,
                                          (const uint8_t **)&in, &available_out,
                                          &out_ptr, NULL)) {
    case BROTLI_DECODER_RESULT_SUCCESS: {
      jing_brotli_decoder_destroy_instance(state);
      r = jing_buffer_append(&b, out, initial_size - available_out, limit);
      if (r > 0) {
        jing_buffer_free(&b);
        mi_free(out);
        jing_err_result(result, r, JING_DEFAULT_FLAG);
        return;
      }
      mi_free(out);
      jing_ptr_result(result, b.ptr, b.used, JING_DEFAULT_FLAG);
      return;
    }
    case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT: {
      r = jing_buffer_append(&b, out, initial_size - available_out, limit);
      if (r > 0) {
        jing_brotli_decoder_destroy_instance(state);
        jing_buffer_free(&b);
        mi_free(out);
        jing_err_result(result, r, JING_DEFAULT_FLAG);
        return;
      }
      available_out = initial_size;
      out_ptr = out;
      break;
    }
    default: {
      jing_brotli_decoder_destroy_instance(state);
      jing_buffer_free(&b);
      mi_free(out);
      jing_err_result(result, ENOTRECOVERABLE, JING_DEFAULT_FLAG);
      return;
    }
    }
  }
}