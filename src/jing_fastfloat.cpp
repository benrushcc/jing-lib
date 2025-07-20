#include "jing_fastfloat.h"
#include "fast_float/fast_float.h"
#include "jing_common.h"

extern "C" {

void jing_parse_double(const char *str, size_t len, jing_result *result) {
  double r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_double_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}

void jing_parse_float(const char *str, size_t len, jing_result *result) {
  float r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_float_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}

void jing_parse_long(const char *str, size_t len, jing_result *result) {
  int64_t r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_long_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}

void jing_parse_int(const char *str, size_t len, jing_result *result) {
  int32_t r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_int_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}

void jing_parse_short(const char *str, size_t len, jing_result *result) {
  int16_t r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_short_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}

void jing_parse_byte(const char *str, size_t len, jing_result *result) {
  int8_t r;
  auto answer = fast_float::from_chars(str, str + len, r);
  if (answer.ec == std::errc()) {
    jing_byte_result(result, r, JING_DEFAULT_FLAG);
  } else {
    jing_err_result(result, static_cast<int>(answer.ec), JING_DEFAULT_FLAG);
  }
}
}
