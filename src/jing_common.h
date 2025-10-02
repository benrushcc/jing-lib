#ifndef JING_COMMON_H
#define JING_COMMON_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

// platform checking
#if defined(_WIN32) || defined(_WIN64)
#define OS_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
#define OS_MACOS 1
#elif defined(__linux__)
#define OS_LINUX 1
#else
#error "Unknown compiler/platform"
#endif

// size checking
static_assert(sizeof(size_t) == 8, "size_t size mismatch");
static_assert(sizeof(void *) == 8, "pointer size mismatch");

// branch prediction optimization
#if defined(__GNUC__) || defined(__clang__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

// symbol control
#if defined(_WIN32)
#define JING_EXPORT_SYMBOL __declspec(dllexport)
#define JING_HIDDEN_SYMBOL
#else
#define JING_EXPORT_SYMBOL __attribute__((visibility("default")))
#define JING_HIDDEN_SYMBOL __attribute__((visibility("hidden")))
#endif

typedef union {
  int8_t byte_val;   // JAVA_BYTE (byte)
  int16_t short_val; // JAVA_SHORT (short)
  int32_t int_val;   // JAVA_INT (int)
  int64_t long_val;  // JAVA_LONG (long)
  float float_val;   // JAVA_FLOAT (float)
  double double_val; // JAVA_DOUBLE (double)
  void *ptr_val;     // ADDRESS
} jing_data;

typedef struct {
  int err;        // errno field
  int flag;       // optional flag field
  size_t len;     // optional pointer length field
  jing_data data; // data field
} jing_result;

#define JING_NO_ERR 0

#define JING_DEFAULT_FLAG 0

JING_HIDDEN_SYMBOL void jing_err_result(jing_result *r, int err, int flag);

JING_HIDDEN_SYMBOL void jing_byte_result(jing_result *r, int8_t value,
                                         int flag);

JING_HIDDEN_SYMBOL void jing_short_result(jing_result *r, int16_t value,
                                          int flag);

JING_HIDDEN_SYMBOL void jing_int_result(jing_result *r, int32_t value,
                                        int flag);

JING_HIDDEN_SYMBOL void jing_long_result(jing_result *r, int64_t value,
                                         int flag);

JING_HIDDEN_SYMBOL void jing_float_result(jing_result *r, float value,
                                          int flag);

JING_HIDDEN_SYMBOL void jing_double_result(jing_result *r, double value,
                                           int flag);

JING_HIDDEN_SYMBOL void jing_ptr_result(jing_result *r, void *value, size_t len,
                                        int flag);

typedef void *(*malloc_func_t)(size_t size);

typedef void *(*realloc_func_t)(void *ptr, size_t size);

typedef void (*free_func_t)(void *ptr);

JING_EXPORT_SYMBOL void *jing_malloc(size_t size);

JING_EXPORT_SYMBOL void jing_free(void *ptr);

JING_EXPORT_SYMBOL void *jing_realloc(void *ptr, size_t size);

JING_EXPORT_SYMBOL void *jing_aligned_alloc(size_t alignment, size_t size);

JING_EXPORT_SYMBOL void jing_aligned_free(void *ptr);

typedef struct {
  void *ptr;
  size_t used;
  size_t len;
  malloc_func_t malloc_func;
  realloc_func_t realloc_func;
  free_func_t free_func;
} jing_buffer;

JING_HIDDEN_SYMBOL int jing_buffer_init(jing_buffer *buffer, size_t init_size,
                                        malloc_func_t mc, realloc_func_t rc,
                                        free_func_t fc);

JING_HIDDEN_SYMBOL int jing_buffer_append(jing_buffer *buffer, void *in,
                                          size_t len, size_t limit);

JING_HIDDEN_SYMBOL void jing_buffer_free(jing_buffer *buffer);

#ifdef JING_ENABLE_TESTS

JING_EXPORT_SYMBOL int jing_test_int(void);

JING_EXPORT_SYMBOL int jing_test_errno(int err);

JING_EXPORT_SYMBOL int jing_test_sum_vargs(int count, ...);

#endif

#endif
