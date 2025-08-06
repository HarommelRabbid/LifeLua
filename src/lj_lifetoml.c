/*
    LifeLua WIP
    TOML library
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdbool.h>
#include <zlib.h>
#include <string.h>

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/tomlc17.h"
#include "include/asprintf.h"

#include "lj_lifeinit.h"

static void push_toml_datum(lua_State *L, toml_datum_t datum);

static void push_toml_array(lua_State *L, toml_datum_t datum) {
    lua_newtable(L);
    for (int i = 0; i < datum.u.arr.size; i++) {
        push_toml_datum(L, datum.u.arr.elem[i]);
        lua_rawseti(L, -2, i + 1);
    }
}

static void push_toml_table(lua_State *L, toml_datum_t datum) {
    lua_newtable(L);
    for (int i = 0; i < datum.u.tab.size; i++) {
        const char *key = datum.u.tab.key[i];
        toml_datum_t val = datum.u.tab.value[i];
        push_toml_datum(L, val);
        lua_setfield(L, -2, key);
    }
}

static void push_toml_datum(lua_State *L, toml_datum_t datum) {
    switch (datum.type) {
        case TOML_STRING:
            lua_pushstring(L, datum.u.s);
            break;
        case TOML_INT64:
            lua_pushinteger(L, datum.u.int64);
            break;
        case TOML_FP64:
            lua_pushnumber(L, datum.u.fp64);
            break;
        case TOML_BOOLEAN:
            lua_pushboolean(L, datum.u.boolean);
            break;
        case TOML_ARRAY:
            push_toml_array(L, datum);
            break;
        case TOML_TABLE:
            push_toml_table(L, datum);
            break;
        case TOML_DATE:
        case TOML_TIME:
        case TOML_DATETIME:
        case TOML_DATETIMETZ: {
            // Convert to ISO 8601 string
            char *buf;
            int y = datum.u.ts.year, mo = datum.u.ts.month, d = datum.u.ts.day;
            int h = datum.u.ts.hour, mi = datum.u.ts.minute, s = datum.u.ts.second;
            int us = datum.u.ts.usec;
            int tz = datum.u.ts.tz;

            if (datum.type == TOML_DATE) {
                asprintf(&buf, "%04d-%02d-%02d", y, mo, d);
            } else if (datum.type == TOML_TIME) {
                asprintf(&buf, "%02d:%02d:%02d.%06d", h, mi, s, us);
            } else { // datetime or datetimetz
                if (tz == 0x8000) { // no tz info
                    asprintf(&buf, "%04d-%02d-%02dT%02d:%02d:%02d.%06d", y, mo, d, h, mi, s, us);
                } else {
                    int tz_h = tz / 60;
                    int tz_m = tz % 60;
                    char sign = (tz >= 0) ? '+' : '-';
                    if (tz < 0) {
                        tz_h = -tz_h;
                        tz_m = -tz_m;
                    }
                    asprintf(&buf, "%04d-%02d-%02dT%02d:%02d:%02d.%06d%c%02d:%02d",
                        y, mo, d, h, mi, s, us, sign, tz_h, tz_m);
                }
            }
            lua_pushstring(L, buf);
            free(buf);
            break;
        }
        default:
            lua_pushnil(L);
            break;
    }
}

static int lua_parse(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);

    toml_result_t res = toml_parse_file_ex(filename);
    if (!res.ok) return luaL_error(L, res.errmsg);

    push_toml_datum(L, res.toptab);

    toml_free(res);
    return 1;
}

static const luaL_Reg toml_lib[] = {
    {"parse", lua_parse},
    {NULL, NULL}
};

LUALIB_API int luaL_opentoml(lua_State *L) {
	luaL_register(L, "toml", toml_lib);
    return 1;
}