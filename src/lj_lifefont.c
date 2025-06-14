/*
    LifeLua WIP
    Font library
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

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

static int lua_loadfont(lua_State *L){
	const char *filename = luaL_checkstring(L, 1);
    Font *font = (Font *)lua_newuserdata(L, sizeof(Font));
	memset(font, 0, sizeof(Font));
	if(file_exists(filename)){
		if(string_ends_with(filename, ".pgf")){
			font->pgf = vita2d_load_custom_pgf(filename);
		}else if((string_ends_with(filename, ".pvf"))){
			font->pvf = vita2d_load_custom_pvf(filename);
		}else if(string_ends_with(filename, ".ttf") || string_ends_with(filename, ".woff")){
			font->font = vita2d_load_font_file(filename);
		}else{
			lua_pushnil(L);
			//return luaL_error(L, "Font file type isn't accepted (must be a .pgf, .pvf, or a .ttf/.woff)");
		}
	}else{
		lua_pushnil(L);
	}
    if (!(font->pgf || font->pvf || font->font)) return luaL_error(L, "Failed to load font: %s", filename);

	luaL_getmetatable(L, "font");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_fontgc(lua_State *L){
	Font *font = (Font *)luaL_checkudata(L, 1, "font");
    if (font->pgf != NULL) {
        vita2d_free_pgf(font->pgf);
    }else if(font->pvf != NULL){
		vita2d_free_pvf(font->pvf);
	}else if(font->font != NULL){
		vita2d_free_font(font->font);
	}
	return 0;
}

static const luaL_Reg font_lib[] = {
    {"load", lua_loadfont},
    {NULL, NULL}
};

LUALIB_API int luaL_openfont(lua_State *L){
	luaL_newmetatable(L, "font");
    lua_pushcfunction(L, lua_fontgc);
    lua_setfield(L, -2, "__gc");

	luaL_register(L, "font", font_lib);
    return 1;
}