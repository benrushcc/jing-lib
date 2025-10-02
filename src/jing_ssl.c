#include "jing_ssl.h"
#include "openssl/crypto.h"
#include "openssl/opensslv.h"

const char *jing_ssl_version(void) { return OpenSSL_version(OPENSSL_VERSION); }

const char *jing_ssl_cflags(void) { return OpenSSL_version(OPENSSL_CFLAGS); }

const char *jing_ssl_built_on(void) {
  return OpenSSL_version(OPENSSL_BUILT_ON);
}