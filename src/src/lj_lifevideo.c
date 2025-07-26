/*
    LifeLua WIP
    Video library
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

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/vitaaudiolib.h"

#include "lj_lifeinit.h"



static const luaL_Reg video_lib[] = {
    {"load", lua_videoload},
    {"output", lua_output},
    {"stop", lua_stop},
    {NULL, NULL}
};

static const luaL_Reg video_methods[] = {
    {"output", lua_output},
    {"stop", lua_stop},
    {"__gc", lua_videogc},
    {NULL, NULL}
};

LUALIB_API int luaL_openvideo(lua_State *L) {
	luaL_newmetatable(L, "video");

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, video_methods);

	luaL_register(L, "video", video_lib);
    return 1;
}