#include "jing_common.h"

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