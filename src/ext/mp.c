#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if LUA_VERSION_NUM == 501
//# define LUA_OK 0
//# define luaL_setfuncs(L,l,u) luaL_register(L,NULL,l)
//# define luaL_newlib(L,l)     (lua_newtable(L), luaL_register(L,NULL,l))
# define luaL_len(L,i)        lua_objlen(L,i)

#include "../lj_lifeinit.h"

static const char *luaL_tolstring (lua_State *L, int idx, size_t *len) {
    if (luaL_callmeta(L, idx, "__tostring")) {
        if (!lua_isstring(L, -1))
            luaL_error(L, "'__tostring' must return a string");
    } else {
        int tt;
        const char *kind;
        switch (lua_type(L, idx)) {
        case LUA_TNUMBER:
            lua_pushfstring(L, "%f", (LUAI_UACNUMBER)lua_tonumber(L, idx));
            break;
        case LUA_TSTRING:
            lua_pushvalue(L, idx); break;
        case LUA_TBOOLEAN:
            lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
            break;
        case LUA_TNIL:
            lua_pushliteral(L, "nil");
            break;
        default: 
            tt = luaL_getmetafield(L, idx, "__name");
            kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) :
                luaL_typename(L, idx);
            lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
            if (tt != LUA_TNIL) lua_remove(L, -2);
        }
    }
    return lua_tolstring(L, -1, len);
}
#endif

#if LUA_VERSION_NUM >= 503
# define lua53_getfield   lua_getfield
# define lua53_geti       lua_geti
# define lua53_tointegerx lua_tointegerx
#else
static int lua53_getfield(lua_State *L, int idx, const char *f)
{ lua_getfield(L, idx, f); return lua_type(L, -1); }
static int lua53_geti(lua_State *L, int idx, int i)
{ lua_rawgeti(L, idx, i); return lua_type(L, -1); }
static void lua_rotate(lua_State *L, int start, int n)
{ int i; for (i = 0; i < n; ++i) lua_insert(L, start+i); }

static lua_Integer lua53_tointegerx(lua_State *L, int idx, int *isint) { 
    lua_Integer i = lua_tointeger(L, idx);
    *isint = i == 0 ? lua_type(L, idx)==LUA_TNUMBER : lua_tonumber(L, idx)==i;
    return i;
}
#endif

typedef signed   long long lmp_I64;
typedef unsigned long long lmp_U64;


/* type info */

#define LMP_TYPE_BOX   "msgpack.Types"
#define LMP_TYPE_FIELD "msgpack.type"

static int Lnull_tostring(lua_State *L)
{ lua_pushliteral(L, "null"); return 1; }

static int Lnull_index(lua_State *L)
{ return luaL_error(L, "attempt to index a msgpack.null value"); }

static int lmp_fetchtable(lua_State *L, const char *name) {
    if (!luaL_newmetatable(L, LMP_TYPE_BOX)) {
        if (lua53_getfield(L, -1, name) != LUA_TNIL) {
            lua_remove(L, -2);
            return 0;
        }
        lua_pop(L, 1);
    }
    lua_createtable(L, 0, 2); /* 1 */
    lua_pushstring(L, name); /* 2 */
    lua_pushvalue(L, -2); /* 1->3 */
    lua_pushvalue(L, -2); /* 2->4 */
    lua_pushvalue(L, -3); /* 2->5 */
    lua_setfield(L, -3, LMP_TYPE_FIELD); /* 5->2 */
    lua_setfield(L, -2, "__name"); /* 4->2 */
    lua_rawset(L, -4); /* 2,3 -> mt */
    lua_remove(L, -2); /* (mt) */
    return 1;
}

static void lmp_pushnull(lua_State *L) {
    lua_newtable(L);
    if (lmp_fetchtable(L, "null")) {
        luaL_Reg libs[] = {
            { "__index",    Lnull_index },
            { "__newindex", Lnull_index },
            { "__tostring", Lnull_tostring },
            { NULL, NULL }
        };
        luaL_setfuncs(L, libs, 0);
    }
    lua_setmetatable(L, -2);
}

static const char *lmp_type(lua_State *L, int idx) {
    const char *r = "";
    if (lua_getmetatable(L, idx)) {
        if (lua53_getfield(L, -1, LMP_TYPE_FIELD) == LUA_TSTRING)
            r = lua_tostring(L, -1);
        lua_pop(L, 2);
    }
    return r;
}

static int lmp_object(lua_State *L, int i, const char *name) {
    int start = i, top = lua_gettop(L);
    for (; i <= top; ++i) {
        if (lua_type(L, i) != LUA_TTABLE) {
            lua_createtable(L, 0, 1);
            lua_pushvalue(L, i);
            lua_setfield(L, -2, "value");
            lua_replace(L, i);
            lmp_fetchtable(L, name);
            lua_setmetatable(L, i);
        } else if (lua_getmetatable(L, i)) {
            lua_pushstring(L, name);
            lua_setfield(L, -2, LMP_TYPE_FIELD);
            lua_pop(L, 1);
        } else {
            lmp_fetchtable(L, name);
            lua_setmetatable(L, i);
        }
    }
    return top - start + 1;
}

static int Lmeta(lua_State *L) {
    const char *types[] = { "null", "False", "True", "int", "uint",
        "float", "double", "string", "binary", "value", "handler",
        "array", "map", "extension", NULL };
    return lmp_object(L, 2, types[luaL_checkoption(L, 1, NULL, types)]);
}

static int Larray(lua_State *L)   { return lmp_object(L, 1, "array"); }
static int Lmap(lua_State *L)     { return lmp_object(L, 1, "map"); }


/* encode */

#define LMP_SSO_SIZE  (sizeof(lmp_HeapBuffer))
#define LMP_MAX_SIZE  (~(unsigned)0>>1)
#define LMP_MAX_STACK 100

typedef struct lmp_HeapBuffer {
    unsigned cap;
    unsigned char *data;
} lmp_HeapBuffer;

typedef struct lmp_Buffer {
    lua_State *L;
    unsigned len : sizeof(unsigned) * CHAR_BIT - 1;
    unsigned sso : 1;
    union {
        lmp_HeapBuffer heap;
        unsigned char buff[LMP_SSO_SIZE];
    } u;
} lmp_Buffer;

#define lmp_data(B)         ((B)->sso ? (B)->u.heap.data : (B)->u.buff)
#define lmp_addchar(B,ch)   (*lmp_prepare(B,1)=(ch),++(B)->len)
#define lmp_addchars(B,s,l) (memcpy(lmp_prepare(B,l),s,l),(B)->len+=(unsigned)(l))

#define lmp_nomem(B)        luaL_error((B)->L, "out of memory")
#define lmp_toobig(B,i,n,l) lmp_error(B,i,0,"%s too large (count=%d)",(n),(int)(l))

static int lmp_encode (lmp_Buffer *B, int idx, int type, int hidx);
static int lmp_pack   (lmp_Buffer *B, int idx, const char *type, int fetch);

static void lmp_resetbuffer(lmp_Buffer *B)
{ if (B->sso) { free(lmp_data(B)); memset(B, 0, sizeof(lmp_Buffer)); } }

static unsigned char *lmp_prepare(lmp_Buffer *B, size_t len) {
    unsigned expected = B->len + (unsigned)len;
    unsigned cap = B->sso ? B->u.heap.cap : LMP_SSO_SIZE;
    if (expected > cap) {
        unsigned newsize = LMP_SSO_SIZE;
        void *newptr, *oldptr = B->sso ? lmp_data(B) : NULL;
        while (newsize < expected && newsize < LMP_MAX_SIZE)
            newsize += newsize >> 1;
        if (newsize < expected) lmp_nomem(B);
        if (!(newptr = realloc(oldptr, newsize))) lmp_nomem(B);
        if (!B->sso) memcpy(newptr, lmp_data(B), B->len);
        B->sso = 1;
        B->u.heap.data = (unsigned char*)newptr;
        B->u.heap.cap  = newsize;
    }
    return lmp_data(B) + B->len;
}

static void lmp_writeuint(lmp_Buffer *B, lmp_U64 v, int len) {
    unsigned char buff[8];
    switch (len) {
    case 8: buff[0] = (v >> 56) & 0xFF;
            buff[1] = (v >> 48) & 0xFF;
            buff[2] = (v >> 40) & 0xFF;
            buff[3] = (v >> 32) & 0xFF; /* FALLTHROUGH */
    case 4: buff[4] = (v >> 24) & 0xFF;
            buff[5] = (v >> 16) & 0xFF; /* FALLTHROUGH */
    case 2: buff[6] = (v >>  8) & 0xFF; /* FALLTHROUGH */
    case 1: buff[7] = (v      ) & 0xFF; /* FALLTHROUGH */
    }
    lmp_addchars(B, buff+8-len, len);
}

static int lmp_calcbytes(lmp_U64 v)
{ return v < 0x100 ? 0 : v < 0x10000 ? 1 : v < 0x100000000 ? 2 : 3; }

static void lmp_prefix(lmp_Buffer *B, lmp_U64 v, int base, int o)
{ lmp_addchar(B, base + o); lmp_writeuint(B, v, 1<<o); }

static void lmp_writeint(lmp_Buffer *B, lmp_U64 v, int uint) {
    if (v < 128) lmp_addchar(B, v & 0xFF);
    else if (uint) lmp_prefix(B, v, 0xCC, lmp_calcbytes(v));
    else if (~v < 32) lmp_addchar(B, v & 0xFF);
    else lmp_prefix(B, v, 0xD0, lmp_calcbytes(v >> 63 ? ~v + 1 : v<<1));
}

static void lmp_writefloat(lmp_Buffer *B, lua_Number n, int len) {
    union {
        float    f32;
        double   f64;
        unsigned u32;
        lmp_U64  u64;
    } u;
    if (len == 4) {
        u.f32 = (float)n; 
        lmp_addchar(B, 0xCA);
        lmp_writeuint(B, u.u32, 4);
    } else {
        u.f64 = (double)n;
        lmp_addchar(B, 0xCB);
        lmp_writeuint(B, u.u64, 8);
    }
}

static void lmp_writestring(lmp_Buffer *B, int base, const char *s, size_t len) {
    if (len < 32 && base == 0xD9) { /* str */
        lmp_addchar(B, (char)(0xA0 + len));
        lmp_addchars(B, s, len);
    } else {
        lmp_prefix(B, len, base, lmp_calcbytes(len));
        lmp_addchars(B, s, len);
    }
}

static void lmp_writeext(lmp_Buffer *B, int type, const char *s, size_t len) {
    unsigned char *buff = lmp_prepare(B, 2);
    int o;
    switch (len) {
    case 1:  buff[0] = 0xD4; break;
    case 2:  buff[0] = 0xD5; break;
    case 4:  buff[0] = 0xD6; break;
    case 8:  buff[0] = 0xD7; break;
    case 16: buff[0] = 0xD8; break;
    default:
      o = lmp_calcbytes(len);
      lmp_prefix(B, len, 0xC7, o);
      lmp_addchar(B, type);
      lmp_addchars(B, s, len);
      return;
    }
    buff[1] = type, B->len += 2;
    lmp_addchars(B, s, len);
}

static int lmp_addfloat(lmp_Buffer *B, int idx, int len)
{ lmp_writefloat(B, lua_tonumber(B->L, idx), len); return 1; }

static int lmp_relindex(int idx, int onstack)
{ return idx > 0 || idx <= LUA_REGISTRYINDEX ? idx : idx - onstack; }

static int lmp_error(lmp_Buffer *B, int idx, const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    lua_pushvfstring(B->L, fmt, l);
    va_end(l);
    lua_replace(B->L, lmp_relindex(idx, 1));
    lua_settop(B->L, idx);
    return 0;
}

static int lmp_chain(lmp_Buffer *B, int idx, int prev, const char *fmt, ...) {
    va_list l;
    va_start(l, fmt);
    lua_pushvfstring(B->L, fmt, l);
    va_end(l);
    lua_pushliteral(B->L, ";\n\t");
    lua_pushvalue(B->L, lmp_relindex(prev, 2));
    lua_concat(B->L, 3);
    lua_replace(B->L, lmp_relindex(idx, 1));
    lua_settop(B->L, idx);
    return 0;
}

static int lmp_addinteger(lmp_Buffer *B, int idx, int uint) {
    int isint;
    lua_Integer i = lua53_tointegerx(B->L, idx, &isint);
    if (!isint)
        return lmp_error(B, idx, "integer expected, got %s",
                luaL_typename(B->L, idx));
    if (uint) lmp_writeint(B, (lmp_U64)i, 1);
    else      lmp_writeint(B, (lmp_U64)i, 0);
    return 1;
}

static int lmp_addstring(lmp_Buffer *B, int idx, int type) {
    size_t len;
    const char *s = lua_tolstring(B->L, idx, &len);
    if (len > LMP_MAX_SIZE) return lmp_toobig(B,idx,"string",(int)len);
    lmp_writestring(B, type, s, len);
    return 1;
}

static int lmp_addext(lmp_Buffer *B, int idx, int fetch) {
    int type, tt, vt;
    size_t len;
    const char *s;
    tt = fetch ? lua53_getfield(B->L, idx, "type") : lua_type(B->L, idx);
    if (tt != LUA_TNUMBER)
        return lmp_error(B, idx,
                "integer expected for extension type, got %s",
                lua_typename(B->L, tt));
    type = (int)lua_tointeger(B->L, fetch ? -1 : idx);
    if (type < -128 || type > 127)
        return lmp_error(B, idx, "invalid extension type: %d", type);
    vt = fetch ? lua53_getfield(B->L, idx, "value") : lua_type(B->L, idx+1);
    if (vt != LUA_TSTRING)
        return lmp_error(B, idx,
                "string expected for extension value, got %s",
                lua_typename(B->L, vt));
    s = lua_tolstring(B->L, fetch ? -1 : idx+1, &len);
    if (len > LMP_MAX_SIZE) return lmp_toobig(B,idx,"extension",(int)len);
    lmp_writeext(B, type, s, len);
    if (fetch) lua_pop(B->L, 2);
    return 1;
}

static int lmp_handlerresult(lmp_Buffer *B, int idx, int top) {
    int r;
    const char *type;
    if ((type = lua_tostring(B->L, top)) == NULL)
        return lmp_error(B, idx, "type expected from handler, got %s",
                luaL_typename(B->L, top));
    r = *type == 'e' ? lmp_addext(B, top+1, 0) : lmp_pack(B, top+1, type, 0);
    if (r ==  0) return lmp_chain(B, idx,top+1, "error from handler");
    if (r == -1) return lmp_error(B, idx, "invalid msgpack.type '%s'", type);
    return (lua_pop(B->L, 3), 1);
}

static int lmp_encoderesult(lmp_Buffer *B, int idx, int r, int hidx) {
    if (r < 0 && hidx == 0)
        return lmp_error(B, idx, "invalid type '%s'", lua_typename(B->L, -r));
    return r;
}

static int lmp_addarray(lmp_Buffer *B, int idx, int hidx) {
    int i, len = (int)luaL_len(B->L, idx);
    int top = lua_gettop(B->L);
    if (top > LMP_MAX_STACK || !lua_checkstack(B->L, 5))
        return lmp_error(B, idx, "array level too deep");
    if (len < 16)
        lmp_addchar(B, 0x90 + len);
    else if (len < 0x10000) {
        lmp_addchar(B, 0xDC);
        lmp_writeuint(B, len, 2);
    } else {
        lmp_addchar(B, 0xDD);
        lmp_writeuint(B, len, 4);
    }
    for (i = 1; i <= len; ++i) {
        int r = lmp_encode(B, top+1, lua53_geti(B->L, idx, i), hidx);
        if ((r = lmp_encoderesult(B, top+1, r, hidx)) < 0) {
            lua_pushvalue(B->L, hidx);
            lua_insert(B->L, -2);
            lua_pushinteger(B->L, i);
            lua_pushvalue(B->L, idx);
            lua_call(B->L, 3, 3);
            r = lmp_handlerresult(B, top+1, top+1);
        }
        if (!r) return lmp_chain(B, idx,top+1, "invalid element '%d' in array", i);
        lua_settop(B->L, top);
    }
    return 1;
}

static void lmp_fixmapszie(lmp_Buffer *B, unsigned off, unsigned count) {
    unsigned len = B->len - off;
    unsigned char *buff;
    lmp_prepare(B, 5);
    buff = lmp_data(B) + off;
    if (count < 16)
        buff[-1] = (char)(0x80 + count);
    else if (count < 0x10000) {
        buff[-1] = 0xDE;
        memmove(buff+2, buff, len);
        B->len = off, lmp_writeuint(B, count, 2); B->len += len;
    } else {
        buff[-1] = 0xDF;
        memmove(buff+4, buff, len);
        B->len = off, lmp_writeuint(B, count, 4); B->len += len;
    }
}

static int lmp_addmap(lmp_Buffer *B, int idx, int hidx) {
    unsigned off = B->len + 1, count = 0;
    int top = lua_gettop(B->L);
    if (top > LMP_MAX_STACK || !lua_checkstack(B->L, 10))
        return lmp_error(B, idx, "map level too deep");
    lmp_addchar(B, 0x80);
    lua_pushnil(B->L);
    for (; lua_next(B->L, idx); ++count) {
        int r = lmp_encode(B, top+1, 0, hidx);
        if ((r = lmp_encoderesult(B, top+1, r, hidx)) < 0) {
            lua_pushvalue(B->L, hidx);
            lua_pushvalue(B->L, top+1);
            lua_pushnil(B->L); /* for key, the key is nil */
            lua_pushvalue(B->L, idx);
            lua_call(B->L, 3, 3);
            r = lmp_handlerresult(B, top+1, top+3);
        }
        if (!r) return lmp_chain(B, idx, top+1, "invalid key in map");
        r = lmp_encode(B, top+2, 0, hidx);
        if ((r = lmp_encoderesult(B, top+2, r, hidx)) < 0) {
            lua_pushvalue(B->L, hidx);
            lua_insert(B->L, -2);
            lua_pushvalue(B->L, top+1);
            lua_pushvalue(B->L, idx);
            lua_call(B->L, 3, 3);
            r = lmp_handlerresult(B, top+2, top+2);
        }
        if (!r)
            return lmp_chain(B, idx,top+2, "invalid value for key '%s' in map",
                    luaL_tolstring(B->L, top+1, NULL));
        lua_settop(B->L, top+1);
    }
    lmp_fixmapszie(B, off, count);
    return 1;
}

static int lmp_addhandler(lmp_Buffer *B, int idx, int fetch) {
    int top = lua_gettop(B->L)+1;
    if (!fetch)
        lua_pushvalue(B->L, idx);
    else if (lua53_getfield(B->L, idx, "pack") == LUA_TNIL)
        return lmp_error(B, idx, "'pack' field expected in handler object");
    if (fetch) lua_pushvalue(B->L, idx);
    lua_call(B->L, fetch, 3);
    return lmp_handlerresult(B, idx, top);
}

static int lmp_check(lmp_Buffer *B, int idx, int fetch, int type) {
    int rt;
    if (!fetch)
        rt = lua_type(B->L, idx);
    else {
        if ((rt = lua53_getfield(B->L, idx, "value")) == LUA_TNIL)
            return lmp_error(B, idx, "'value' field expected in wrapper object");
        lua_replace(B->L, idx);
    }
    return !type || rt == type ? 1 :
        lmp_error(B, idx, "%s expected, got %s",
                lua_typename(B->L, type), luaL_typename(B->L, idx));
}

static int lmp_pack(lmp_Buffer *B, int idx, const char *type, int fetch) {
#define check(t,c) (lmp_check(B,idx,fetch,t) && (c))
    switch (*type) {
    case 'n': lmp_addchar(B,0xC0); return 1;
    case 'F': lmp_addchar(B,0xC2); return 1;
    case 'T': lmp_addchar(B,0xC3); return 1;
    case 'i': return check(LUA_TNUMBER, lmp_addinteger(B,idx,0));
    case 'u': return check(LUA_TNUMBER, lmp_addinteger(B,idx,1));
    case 'f': return check(LUA_TNUMBER, lmp_addfloat(B,idx,4));
    case 'd': return check(LUA_TNUMBER, lmp_addfloat(B,idx,8));
    case 's': return check(LUA_TSTRING, lmp_addstring(B,idx,0xD9));
    case 'b': return check(LUA_TSTRING, lmp_addstring(B,idx,0xC4));
    case 'v': return check(0,           lmp_encode(B, idx, 0, 0));
    case 'h': return lmp_addhandler(B, idx, 1);
    case 'a': return lmp_addarray(B, idx, 0);
    case 'm': return lmp_addmap(B, idx, 0);
    case 'e': return lmp_addext(B, idx, 1);
    }
#undef check
    return -1;
}

static int lmp_addnumber(lmp_Buffer *B, int idx) {
    int isint;
    lua_Integer i = lua53_tointegerx(B->L, idx, &isint);
    if (isint)
        lmp_writeint(B, (lmp_U64)i, 0);
    else {
        lua_Number n = lua_tonumber(B->L, idx);
        int float_ok = (lua_Number)(float)n == n;
        if (float_ok) lmp_writefloat(B, n, 4);
        else          lmp_writefloat(B, n, 8);
    }
    return 1;
}

static int lmp_addtable(lmp_Buffer *B, int idx, int hidx) {
    const char *type = lmp_type(B->L, idx);
    int r = lmp_pack(B, idx, type, 1);
    if (r == 0 || r == 1) return r;
    return luaL_len(B->L, idx) > 0 ? lmp_addarray(B, idx, hidx) : lmp_addmap(B, idx, hidx);
}

static int lmp_encode(lmp_Buffer *B, int idx, int type, int hidx) {
    switch (type ? type : (type = lua_type(B->L, idx))) {
    case LUA_TNONE:
    case LUA_TNIL:      lmp_addchar(B, 0xC0); return 1;
    case LUA_TBOOLEAN:  lmp_addchar(B, 0xC2+lua_toboolean(B->L,idx)); return 1;
    case LUA_TNUMBER:   return lmp_addnumber(B, idx);
    case LUA_TSTRING:   return lmp_addstring(B, idx, 0xD9);
    case LUA_TFUNCTION: return lmp_addhandler(B, idx, 0);
    case LUA_TTABLE:    return lmp_addtable(B, idx, hidx);
    }
    return -type;
}

static int Lencode_aux(lua_State *L) {
    lmp_Buffer *B = (lmp_Buffer*)lua_touserdata(L, 1);
    int i, top = lua_gettop(L);
    B->L = L;
    for (i = 2; i <= top; ++i) {
        int r = lmp_encode(B, i, 0, 0);
        if (!(r = lmp_encoderesult(B, i, r, 0)))
            return luaL_error(L, "bad argument to #%d: %s",
                    i-1, lua_tostring(L, i));
    }
    lua_pushlstring(L, (const char*)lmp_data(B), B->len);
    return 1;
}

static int Lencode(lua_State *L) {
    lmp_Buffer B;
    int r;
    memset(&B, 0, sizeof(B));
    lua_pushcfunction(L, Lencode_aux);
    lua_insert(L, 1);
    lua_pushlightuserdata(L, &B);
    lua_insert(L, 2);
    r = lua_pcall(L, lua_gettop(L)-1, 1, 0) == LUA_OK;
    lmp_resetbuffer(&B);
    return r ? 1 : luaL_error(L, "%s", lua_tostring(L, -1));
}

static int Lencoder_aux(lua_State *L) {
    lmp_Buffer *B = (lmp_Buffer*)lua_touserdata(L, 1);
    int i, top = lua_gettop(L);
    B->L = L;
    for (i = 3; i <= top; ++i) {
        int r = lmp_encode(B, i, 0, 2);
        if ((r = lmp_encoderesult(B, i, r, 2)) < 0) {
            lua_pushvalue(B->L, 2);
            lua_pushvalue(B->L, i);
            lua_call(B->L, 1, 3);
            r = lmp_handlerresult(B, i, top+1);
        }
        if (r == 0) return luaL_error(L, "bad argument to #%d: %s",
                i-2, lua_tostring(L, i));
    }
    lua_pushlstring(L, (const char*)lmp_data(B), B->len);
    return 1;
}

static int Lencoder(lua_State *L) {
    lmp_Buffer B;
    int r;
    memset(&B, 0, sizeof(B));
    lua_pushcfunction(L, Lencoder_aux);
    lua_insert(L, 1);
    lua_pushlightuserdata(L, &B);
    lua_insert(L, 2);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 3);
    r = lua_pcall(L, lua_gettop(L)-1, 1, 0) == LUA_OK;
    lmp_resetbuffer(&B);
    return r ? 1 : luaL_error(L, "%s", lua_tostring(L, -1));
}

static int Lnewencoder(lua_State *L) {
    if (lua_isnoneornil(L, 1))
        lua_pushcfunction(L, Lencode);
    else {
        lua_pushvalue(L, 1);
        lua_pushcclosure(L, Lencoder, 1);
    }
    return 1;
}


/* decode */

typedef struct lmp_Slice {
    const char *p, *e, *s;
    lua_State  *L;
    int         ext;
} lmp_Slice;

#define lmp_readsize(S,l)  ((int)lmp_readuint(S,(int)l,"size"))
#define lmp_readluint(S,l) ((lua_Integer)lmp_readuint(S,l,"uint"))
#define lmp_readlint(S,l)  ((lua_Integer)lmp_u2s(lmp_readuint(S,l,"int"),l))

static void lmp_decode(lmp_Slice *S);

static size_t lmp_off(lmp_Slice *S) { return S->p - S->s + 1; }
static size_t lmp_len(lmp_Slice *S) { return S->e - S->p; }

static void lmp_ensure(lmp_Slice *S, size_t len, const char *tname) {
    size_t rem = lmp_len(S);
    if (rem >= len) return;
    luaL_error(S->L, "invalid %s at offset %d: "
            "%d bytes expected, got %d bytes",
            tname, (int)lmp_off(S), (int)len, (int)rem);
}

static void lmp_pushstring(lmp_Slice *S, size_t len)
{ lmp_ensure(S, len, "string"); lua_pushlstring(S->L,S->p,len); S->p += len; }

static void lmp_checkend(lmp_Slice *S, const char *tname) {
    if (S->p < S->e) return;
    luaL_error(S->L, "unexpected end of message at offset %d: %s expected",
            (int)lmp_off(S), tname);
}

static lmp_U64 lmp_readuint(lmp_Slice *S, int len, const char *tname) {
    lmp_U64 r = 0;
    lmp_ensure(S, len, tname);
#define ch(i) ((lmp_U64)(unsigned char)S->p[i])
    switch (len) {
    case 8: r |= ch(0) << 56 | ch(1) << 48
              |  ch(2) << 40 | ch(3) << 32; S->p += 4; /* FALLTHROUGH */
    case 4: r |= ch(0) << 24 | ch(1) << 16; S->p += 2; /* FALLTHROUGH */
    case 2: r |= ch(0) << 8;                S->p += 1; /* FALLTHROUGH */
    case 1: r |= ch(0);                     S->p += 1;
    }
#undef ch
    return r;
}

static lmp_I64 lmp_u2s(lmp_U64 v, int len) {
    const lmp_I64 m = 1LL << (len*8 - 1);
    if (len == 8 || !(v & (lmp_U64)m)) return (lmp_I64)v;
    v = v & ((1ULL << len*8) - 1);
    return (lmp_I64)(v^m) - m;
}

static lua_Number lmp_readfloat(lmp_Slice *S, int len) {
    union {
        float    f32;
        double   f64;
        lmp_U64  u64;
        unsigned u32;
    } u;
    if (len == 4) {
        u.u32 = (unsigned)lmp_readuint(S, len, "float");
        return (lua_Number)u.f32;
    } else {
        u.u64 = lmp_readuint(S, len, "float");
        return (lua_Number)u.f64;
    }
}

static void lmp_pushext(lmp_Slice *S, int fix, size_t len) {
    int type;
    lmp_ensure(S, len+1, "extension");
    if (!fix) len = lmp_readsize(S, len), lmp_ensure(S, len, "extension");
    type = (int)(signed char)*S->p++;
    lua_pushinteger(S->L, type);
    lua_pushlstring(S->L, S->p, len);
    S->p += len;
    if (S->ext) {
        lua_pushvalue(S->L, S->ext);
        lua_pushvalue(S->L, -3);
        lua_pushvalue(S->L, -3);
        lua_call(S->L, 2, 1);
        if (!lua_isnil(S->L, -1)) {
            lua_insert(S->L, -3), lua_pop(S->L, 2);
            return;
        }
        lua_pop(S->L, 1);
    }
    lua_createtable(S->L, 0, 2);
    lua_rotate(S->L, -3, 1);
    lua_setfield(S->L, -3, "value");
    lua_setfield(S->L, -2, "type");
}

static void lmp_pusharray(lmp_Slice *S, int count) {
    int i;
    lua_createtable(S->L, count, 0);
    for (i = 0; i < count; ++i) {
        lmp_checkend(S, "array element"); lmp_decode(S);
        lua_rawseti(S->L, -2, (lua_Integer)i+1);
    }
}

static void lmp_pushmap(lmp_Slice *S, int count) {
    int i;
    lua_createtable(S->L, 0, count);
    for (i = 0; i < count; ++i) {
        lmp_checkend(S, "map key");   lmp_decode(S);
        lmp_checkend(S, "map value"); lmp_decode(S);
        lua_rawset(S->L, -3);
    }
}

static void lmp_decode(lmp_Slice *S) {
    int ch = *S->p++ & 0xFF;
    switch (ch) {
    case 0x80: case 0x81: case 0x82: case 0x83:
    case 0x84: case 0x85: case 0x86: case 0x87:
    case 0x88: case 0x89: case 0x8A: case 0x8B:
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        lmp_pushmap(S, ch - 0x80); break;
    case 0x90: case 0x91: case 0x92: case 0x93:
    case 0x94: case 0x95: case 0x96: case 0x97:
    case 0x98: case 0x99: case 0x9A: case 0x9B:
    case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        lmp_pusharray(S, ch - 0x90); break;
    case 0xA0: case 0xA1: case 0xA2: case 0xA3:
    case 0xA4: case 0xA5: case 0xA6: case 0xA7:
    case 0xA8: case 0xA9: case 0xAA: case 0xAB:
    case 0xAC: case 0xAD: case 0xAE: case 0xAF:
    case 0xB0: case 0xB1: case 0xB2: case 0xB3:
    case 0xB4: case 0xB5: case 0xB6: case 0xB7:
    case 0xB8: case 0xB9: case 0xBA: case 0xBB:
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        lmp_pushstring(S, (size_t)ch - 0xA0); break;
    case 0xC0: lua_pushnil(S->L);         break;
    case 0xC2: lua_pushboolean(S->L, 0);  break;
    case 0xC3: lua_pushboolean(S->L, 1);  break;
    case 0xC4: case 0xD9: lmp_pushstring(S, lmp_readsize(S,1)); break;
    case 0xC5: case 0xDA: lmp_pushstring(S, lmp_readsize(S,2)); break;
    case 0xC6: case 0xDB: lmp_pushstring(S, lmp_readsize(S,4)); break;
    case 0xC7: lmp_pushext(S, 0, 1); break;
    case 0xC8: lmp_pushext(S, 0, 2); break;
    case 0xC9: lmp_pushext(S, 0, 4); break;
    case 0xCA: lua_pushnumber(S->L, lmp_readfloat(S, 4)); break;
    case 0xCB: lua_pushnumber(S->L, lmp_readfloat(S, 8)); break;
    case 0xCC: lua_pushinteger(S->L, lmp_readluint(S, 1)); break;
    case 0xCD: lua_pushinteger(S->L, lmp_readluint(S, 2)); break;
    case 0xCE: lua_pushinteger(S->L, lmp_readluint(S, 4)); break;
    case 0xCF: lua_pushinteger(S->L, lmp_readluint(S, 8)); break;
    case 0xD0: lua_pushinteger(S->L, lmp_readlint(S, 1)); break;
    case 0xD1: lua_pushinteger(S->L, lmp_readlint(S, 2)); break;
    case 0xD2: lua_pushinteger(S->L, lmp_readlint(S, 4)); break;
    case 0xD3: lua_pushinteger(S->L, lmp_readlint(S, 8)); break;
    case 0xD4: lmp_pushext(S, 1,  1); break;
    case 0xD5: lmp_pushext(S, 1,  2); break;
    case 0xD6: lmp_pushext(S, 1,  4); break;
    case 0xD7: lmp_pushext(S, 1,  8); break;
    case 0xD8: lmp_pushext(S, 1, 16); break;
    case 0xDC: lmp_pusharray(S, lmp_readsize(S, 2)); break;
    case 0xDD: lmp_pusharray(S, lmp_readsize(S, 4)); break;
    case 0xDE: lmp_pushmap(S, lmp_readsize(S, 2)); break;
    case 0xDF: lmp_pushmap(S, lmp_readsize(S, 4)); break;
    default:
        if      (ch < 0x80) lua_pushinteger(S->L, ch);
        else if (ch > 0xDF) lua_pushinteger(S->L, (lua_Integer)ch - 256);
        else luaL_error(S->L, "invalid char '%d' at offset %d", ch, lmp_off(S));
    }
}

static size_t lmp_posrelat(lua_Integer pos, size_t len) {
  if (pos > 0) return (size_t)pos;
  else if (pos == 0) return 1;
  else if (pos < -(lua_Integer)len) return 1;
  else return len + (size_t)pos + 1;
}

static int Ldecode(lua_State *L) {
    lmp_Slice S;
    size_t len;
    const char *s = luaL_checklstring(L, 1, &len);
    size_t i = lmp_posrelat(luaL_optinteger(L, 2, 1), len);
    size_t j = lmp_posrelat(luaL_optinteger(L, 3, -1), len);
    if (i > j || i > len) return 0;
    lua_settop(L, 4);
    S.s = s;
    S.e = s + j;
    S.p = s + (i ? i : 1) - 1;
    S.L = L;
    S.ext = lua_isnoneornil(L, 4) ? 0 : 4;
    lmp_decode(&S);
    if (S.p >= S.e) return 1;
    lua_pushinteger(L, lmp_off(&S));
    return 2;
}


/* entry point */

static int Ltohex(lua_State *L) {
    size_t i, len;
    const char *s = luaL_checklstring(L, 1, &len);
    const char *hexa = "0123456789ABCDEF";
    char hex[4] = "XX ";
    luaL_Buffer lb;
    luaL_buffinit(L, &lb);
    for (i = 0; i < len; ++i) {
        unsigned int ch = s[i] & 0xFF;
        hex[0] = hexa[(ch>>4)&0xF];
        hex[1] = hexa[(ch   )&0xF];
        if (i == len-1) hex[2] = '\0';
        luaL_addstring(&lb, hex);
    }
    luaL_pushresult(&lb);
    return 1;
}

static int Lfromhex(lua_State *L) {
    size_t i, len;
    const char *s = luaL_checklstring(L, 1, &len);
    luaL_Buffer lb;
    int curr = 0, idx = 0, num;
    luaL_buffinit(L, &lb);
    for (i = 0; i < len; ++i) {
        switch (num = s[i]) {
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9': num -= '0'; break;
        case 'A': case 'a': num  =  10; break;
        case 'B': case 'b': num  =  11; break;
        case 'C': case 'c': num  =  12; break;
        case 'D': case 'd': num  =  13; break;
        case 'E': case 'e': num  =  14; break;
        case 'F': case 'f': num  =  15; break;
        default: continue;
        }
        curr = curr<<4 | num;
        if (++idx % 2 == 0) luaL_addchar(&lb, curr), curr = 0;
    }
    luaL_pushresult(&lb);
    return 1;
}

LUALIB_API int luaL_openmp(lua_State *L) {
    luaL_Reg libs[] = {
        { "null", NULL },
#define ENTRY(name) { #name, L##name }
        ENTRY(array),
        ENTRY(map),
        ENTRY(meta),
        ENTRY(encode),
        ENTRY(newencoder),
        ENTRY(decode),
        ENTRY(fromhex),
        ENTRY(tohex),
#undef  ENTRY
        { NULL, NULL }
    };
#if LUA_VERSION_NUM >= 502
    luaL_newlib(L, libs);
#else
    luaL_register(L, "msgpack", libs);
#endif
    lmp_pushnull(L), lua_setfield(L, -2, "null");
    return 1;
}

/* cc: flags+='-march=native -O3 -Wextra -pedantic --coverage'
 * unixcc: flags+='-shared -fPIC ' output='mp.so'
 * maccc: flags+='-undefined dynamic_lookup'
 * win32cc: flags+='-mdll -DLUA_BUILD_AS_DLL ' libs+='-llua54' output='mp.dll' */
