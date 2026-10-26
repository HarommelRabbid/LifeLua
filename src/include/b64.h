#ifndef CORDLESS_UTIL_B64_H
#define CORDLESS_UTIL_B64_H

#include <stddef.h>

// Standard base64 encode, NUL-terminated. out must hold 4*ceil(len/3)+1 bytes.
size_t b64_encode(const unsigned char *in, size_t len, char *out);

// base64url encode (+/ -> -_, no padding). Same buffer sizing.
size_t b64url_encode(const unsigned char *in, size_t len, char *out);

// Standard base64 decode. out must hold 3*ceil(len/4) bytes. Returns length or -1.
int b64_decode(const char *in, unsigned char *out, size_t out_cap);

#endif  // CORDLESS_UTIL_B64_H