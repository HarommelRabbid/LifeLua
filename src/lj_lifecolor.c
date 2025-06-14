/*
    LifeLua WIP
    Color library
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

static int lua_newcolor(lua_State *L) {
	unsigned int r = luaL_checkinteger(L, 1);
	unsigned int g = luaL_checkinteger(L, 2);
	unsigned int b = luaL_checkinteger(L, 3);
	unsigned int a = luaL_optinteger(L, 4, 255);
	Color *color = (Color *)lua_newuserdata(L, sizeof(Color));
	color->color = RGBA8(CLAMP(r, 0, 255), CLAMP(g, 0, 255), CLAMP(b, 0, 255), CLAMP(a, 0, 255));
	luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_colorr(lua_State *L){
	Color *color = (Color *)luaL_checkudata(L, 1, "color");
	if(lua_isnone(L, 2)){
		lua_pushnumber(L, color->color & 0xFF);
	}else if(lua_isnumber(L, 2)){
		unsigned int r = luaL_checkinteger(L, 2);
		Color *res = (Color *)lua_newuserdata(L, sizeof(Color));
		res->color = RGBA8(CLAMP(r, 0, 255), (color->color >> 8) & 0xFF, (color->color >> 16) & 0xFF, (color->color >> 24) & 0xFF);
		luaL_getmetatable(L, "color");
    	lua_setmetatable(L, -2);
	}
	return 1;
}

static int lua_colorg(lua_State *L){
	Color *color = (Color *)luaL_checkudata(L, 1, "color");
	if(lua_isnone(L, 2)){
		lua_pushnumber(L, (color->color >> 8) & 0xFF);
	}else if(lua_isnumber(L, 2)){
		unsigned int g = luaL_checkinteger(L, 2);
		Color *res = (Color *)lua_newuserdata(L, sizeof(Color));
		res->color = RGBA8(color->color & 0xFF, CLAMP(g, 0, 255), (color->color >> 16) & 0xFF, (color->color >> 24) & 0xFF);
		luaL_getmetatable(L, "color");
    	lua_setmetatable(L, -2);
	}
	return 1;
}

static int lua_colorb(lua_State *L){
	Color *color = (Color *)luaL_checkudata(L, 1, "color");
	if(lua_isnone(L, 2)){
		lua_pushnumber(L, (color->color >> 16) & 0xFF);
	}else if(lua_isnumber(L, 2)){
		unsigned int b = luaL_checkinteger(L, 2);
		Color *res = (Color *)lua_newuserdata(L, sizeof(Color));
		res->color = RGBA8(color->color & 0xFF, (color->color >> 8) & 0xFF, CLAMP(b, 0, 255), (color->color >> 24) & 0xFF);
		luaL_getmetatable(L, "color");
    	lua_setmetatable(L, -2);
	}
	return 1;
}

static int lua_colora(lua_State *L){
	Color *color = (Color *)luaL_checkudata(L, 1, "color");
	if(lua_isnone(L, 2)){
		lua_pushnumber(L, (color->color >> 24) & 0xFF);
	}else if (lua_isnumber(L, 2)){
		unsigned int a = luaL_checkinteger(L, 2);
		Color *res = (Color *)lua_newuserdata(L, sizeof(Color));
		res->color = RGBA8(color->color & 0xFF, (color->color >> 8) & 0xFF, (color->color >> 16) & 0xFF, CLAMP(a, 0, 255));
		luaL_getmetatable(L, "color");
    	lua_setmetatable(L, -2);
	}
	return 1;
}

static int lua_coloradd(lua_State *L){
	Color *color1 = (Color *)luaL_checkudata(L, 1, "color");
	Color *color2 = (Color *)luaL_checkudata(L, 2, "color");

	Color *color = (Color *)lua_newuserdata(L, sizeof(Color));
	color->color = RGBA8(
		CLAMP((color1->color & 0xFF)+(color2->color & 0xFF), 0, 255), //r
		CLAMP(((color1->color >> 8) & 0xFF)+((color2->color >> 8) & 0xFF), 0, 255), //g
		CLAMP(((color1->color >> 16) & 0xFF)+((color2->color >> 16) & 0xFF), 0, 255), //b
		CLAMP(((color1->color >> 24) & 0xFF)+((color2->color >> 24) & 0xFF), 0, 255));
	luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_colorsub(lua_State *L){
	Color *color1 = (Color *)luaL_checkudata(L, 1, "color");
	Color *color2 = (Color *)luaL_checkudata(L, 2, "color");

	Color *color = (Color *)lua_newuserdata(L, sizeof(Color));
	color->color = RGBA8(
		CLAMP((color1->color & 0xFF)-(color2->color & 0xFF), 0, 255), //r
		CLAMP(((color1->color >> 8) & 0xFF)-((color2->color >> 8) & 0xFF), 0, 255), //g
		CLAMP(((color1->color >> 16) & 0xFF)-((color2->color >> 16) & 0xFF), 0, 255), //b
		CLAMP(((color1->color >> 24) & 0xFF)-((color2->color >> 24) & 0xFF), 0, 255));
	luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_coloreq(lua_State *L){
	Color *color1 = (Color *)luaL_checkudata(L, 1, "color");
	Color *color2 = (Color *)luaL_checkudata(L, 2, "color");
	lua_pushboolean(L, color1->color == color2->color);
	return 1;
}

static int lua_colorblend(lua_State *L) {
    Color *c1 = (Color *)luaL_checkudata(L, 1, "color");
    Color *c2 = (Color *)luaL_checkudata(L, 2, "color");

    Color *result = (Color *)lua_newuserdata(L, sizeof(Color));
    result->color = RGBA8(
        ((c1->color & 0xFF) + (c2->color & 0xFF)) / 2,               // R
        (((c1->color >> 8) & 0xFF) + ((c2->color >> 8) & 0xFF)) / 2, // G
        (((c1->color >> 16) & 0xFF) + ((c2->color >> 16) & 0xFF)) / 2, // B
        (((c1->color >> 24) & 0xFF) + ((c2->color >> 24) & 0xFF)) / 2  // A
    );

    luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_colormix(lua_State *L) {
    Color *color1 = (Color *)luaL_checkudata(L, 1, "color");
    Color *color2 = (Color *)luaL_checkudata(L, 2, "color");
    float p1 = luaL_checknumber(L, 3);
    float p2 = luaL_checknumber(L, 4);

    float sum = p1 + p2;
    if (sum <= 0.0f) sum = 1.0f; // avoid division by zero

    float f1 = p1 / sum;
    float f2 = p2 / sum;

    Color *result = (Color *)lua_newuserdata(L, sizeof(Color));
    result->color = RGBA8(
        CLAMP((int)((color1->color & 0xFF) * f1 + (color2->color & 0xFF) * f2), 0, 255),
        CLAMP((int)(((color1->color >> 8) & 0xFF) * f1 + ((color2->color >> 8) & 0xFF) * f2), 0, 255),
        CLAMP((int)(((color1->color >> 16) & 0xFF) * f1 + ((color2->color >> 16) & 0xFF) * f2), 0, 255),
        CLAMP((int)(((color1->color >> 24) & 0xFF) * f1 + ((color2->color >> 24) & 0xFF) * f2), 0, 255)
    );

    luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
    return 1;
}

static const luaL_Reg color_lib[] = {
    {"new", lua_newcolor},
	{"r", lua_colorr},
	{"g", lua_colorg},
	{"b", lua_colorb},
	{"a", lua_colora},
	{"add", lua_coloradd},
	{"sub", lua_colorsub},
	{"blend", lua_colorblend},
	{"mix", lua_colormix},
    {NULL, NULL}
};

static const luaL_Reg color_methods[] = {
	{"r", lua_colorr},
	{"g", lua_colorg},
	{"b", lua_colorb},
	{"a", lua_colora},
	{"add", lua_coloradd},
	{"sub", lua_colorsub},
	{"blend", lua_colorblend},
	{"mix", lua_colormix},
	{"__add", lua_coloradd},
	{"__sub", lua_colorsub},
	{"__eq", lua_coloreq},
    {NULL, NULL}
};

void luaL_opencolor(lua_State *L) {
	luaL_newmetatable(L, "color");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_openlib(L, NULL, color_methods, 0);

	luaL_openlib(L, "color", color_lib, 0);
}