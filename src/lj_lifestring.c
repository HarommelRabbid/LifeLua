/*
    LifeLua WIP
    String library extension
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
#include <libgen.h>

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>

#include "lj_lifeinit.h"

static int lua_startswith(lua_State *L){
    const char *s1 = luaL_checkstring(L, 1);
    const char *s2 = luaL_checkstring(L, 2);
    bool insensitive = lua_toboolean(L, 3);
    if(insensitive) lua_pushboolean(L, strncasecmp(s1, s2, strlen(s2)) == 0);
    else lua_pushboolean(L, strncmp(s1, s2, strlen(s2)) == 0);
    return 1;
}

static int lua_endswith(lua_State *L){
    size_t l1, l2;
    const char *s1 = luaL_checklstring(L, 1, &l1);
    const char *s2 = luaL_checklstring(L, 2, &l2);
    bool insensitive = lua_toboolean(L, 3);

    const char *end = s1 + l1 - l2;

    lua_pushboolean(L, (l2 <= l1) && (insensitive ? strncasecmp(end, s2, l2) == 0 : strncmp(end, s2, l2) == 0));
    return 1;
}

static int lua_split(lua_State *L){
    const char *string = luaL_checkstring(L, 1);
    const char *delimiter = luaL_optstring(L, 2, " ");
    char *str = strdup(string);
    if (!str) return luaL_error(L, "Out of memory");
    char *token = strtok(str, delimiter);
    lua_newtable(L);
    for(int i = 1; token != NULL; i++){
        lua_pushstring(L, token);
        lua_rawseti(L, -2, i);
        token = strtok(NULL, delimiter);
    }
    free(str);
    return 1;
}

// string_methods not needed since they have the same functions as string_lib
static const luaL_Reg string_lib[] = {
    {"startswith", lua_startswith},
    {"endswith", lua_endswith},
    {"split", lua_split},
    {NULL, NULL}
};

LUALIB_API int luaL_extendstring(lua_State *L){
    luaL_register(L, "string", string_lib);

    // Push dummy string and get its metatable
    lua_pushliteral(L, "");
    lua_getmetatable(L, -1);

    // At this point, stack top is the metatable
    lua_getfield(L, -1, "__index");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
    }

    luaL_register(L, NULL, string_lib);
    return 1;
}