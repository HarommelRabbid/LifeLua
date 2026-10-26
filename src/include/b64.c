#include "b64.h"

static const char ENC_STD[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static size_t encode_impl(const unsigned char *in, size_t len, char *out,
                          int urlsafe)
{
    size_t o = 0;
    size_t i = 0;
    while (i + 3 <= len) {
        unsigned v = (in[i] << 16) | (in[i+1] << 8) | in[i+2];
        out[o++] = ENC_STD[(v >> 18) & 0x3F];
        out[o++] = ENC_STD[(v >> 12) & 0x3F];
        out[o++] = ENC_STD[(v >> 6)  & 0x3F];
        out[o++] = ENC_STD[v & 0x3F];
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        unsigned v = in[i] << 16;
        out[o++] = ENC_STD[(v >> 18) & 0x3F];
        out[o++] = ENC_STD[(v >> 12) & 0x3F];
        if (!urlsafe) { out[o++] = '='; out[o++] = '='; }
    } else if (rem == 2) {
        unsigned v = (in[i] << 16) | (in[i+1] << 8);
        out[o++] = ENC_STD[(v >> 18) & 0x3F];
        out[o++] = ENC_STD[(v >> 12) & 0x3F];
        out[o++] = ENC_STD[(v >> 6)  & 0x3F];
        if (!urlsafe) out[o++] = '=';
    }
    if (urlsafe) {
        for (size_t k = 0; k < o; k++) {
            if (out[k] == '+') out[k] = '-';
            else if (out[k] == '/') out[k] = '_';
        }
    }
    out[o] = '\0';
    return o;
}

size_t b64_encode(const unsigned char *in, size_t len, char *out)
{
    return encode_impl(in, len, out, 0);
}

size_t b64url_encode(const unsigned char *in, size_t len, char *out)
{
    return encode_impl(in, len, out, 1);
}

static int dec_val(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+' || c == '-') return 62;
    if (c == '/' || c == '_') return 63;
    return -1;
}

int b64_decode(const char *in, unsigned char *out, size_t out_cap)
{
    int quad[4];
    int qn = 0;
    size_t o = 0;
    for (const char *p = in; *p; p++) {
        if (*p == '=' ) break;
        int v = dec_val(*p);
        if (v < 0) continue;  // skip whitespace/newlines
        quad[qn++] = v;
        if (qn == 4) {
            unsigned x = (quad[0] << 18) | (quad[1] << 12) | (quad[2] << 6) | quad[3];
            if (o + 3 > out_cap) return -1;
            out[o++] = (x >> 16) & 0xFF;
            out[o++] = (x >> 8)  & 0xFF;
            out[o++] = x & 0xFF;
            qn = 0;
        }
    }
    if (qn == 2) {
        unsigned x = (quad[0] << 18) | (quad[1] << 12);
        if (o + 1 > out_cap) return -1;
        out[o++] = (x >> 16) & 0xFF;
    } else if (qn == 3) {
        unsigned x = (quad[0] << 18) | (quad[1] << 12) | (quad[2] << 6);
        if (o + 2 > out_cap) return -1;
        out[o++] = (x >> 16) & 0xFF;
        out[o++] = (x >> 8) & 0xFF;
    } else if (qn != 0) {
        return -1;  // leftover single char: malformed
    }
    return (int)o;
}