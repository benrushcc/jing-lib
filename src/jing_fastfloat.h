#ifndef JING_FASTFLOAT_H
#define JING_FASTFLOAT_H

#include "jing_common.h"

#ifdef __cplusplus
extern "C" {
#endif

JING_EXPORT_SYMBOL void jing_parse_double(const char *str, size_t len,
                                          jing_result *result);
JING_EXPORT_SYMBOL void jing_parse_float(const char *str, size_t len,
                                         jing_result *result);
JING_EXPORT_SYMBOL void jing_parse_long(const char *str, size_t len,
                                        jing_result *result);
JING_EXPORT_SYMBOL void jing_parse_int(const char *str, size_t len,
                                       jing_result *result);
JING_EXPORT_SYMBOL void jing_parse_short(const char *str, size_t len,
                                         jing_result *result);
JING_EXPORT_SYMBOL void jing_parse_byte(const char *str, size_t len,
                                        jing_result *result);

#ifdef __cplusplus
}
#endif

#endif