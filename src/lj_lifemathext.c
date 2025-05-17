/*
    LifeLua WIP
    Math library extension
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

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/ftpvita.h"

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>
#define str(str) #str
#define luaL_pushglobalint(L, value) do { lua_pushinteger(L, value); lua_setglobal (L, str(value)); } while(0)
#define luaL_pushglobalint_as(L, value, var) do { lua_pushinteger(L, value); lua_setglobal (L, var); } while(0)
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

static int lua_lerp(lua_State *L){
	double val = luaL_checknumber(L, 1);
	double min = luaL_checknumber(L, 2);
	double max = luaL_checknumber(L, 3);
	lua_pushnumber(L, lerp(val, min, max));
	return 1;
}

static const struct luaL_Reg math_lib[] = {
	{"lerp", lua_lerp},
    {NULL, NULL}
};

void luaL_extendmath(lua_State *L) {
	luaL_dostring(L, "function math.range(num, min, max) return math.min(math.max(num, min), max) end");
	lua_getglobal(L, "math");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setglobal(L, "math");
		lua_getglobal(L, "math");
	}

	luaL_setfuncs(L, math_lib, 0); // extending the math library
	lua_pop(L, 1);
}