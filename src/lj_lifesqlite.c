/*
    LifeLua WIP
    sqlite3 library
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
#include "include/sqlite3.h"

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

int callback_results = 1;
static int sqlite_callback(void *data, int argc, char **argv, char **azColName){
	lua_State *L = (lua_State*)data;
	lua_pushnumber(L, callback_results++);
	lua_newtable(L);
	for (int i = 0; i < argc; i++) {
		lua_pushstring(L,  azColName[i]);
		if (argv[i] != NULL)
			lua_pushstring(L, argv[i]);
		else
			lua_pushnil(L);
		lua_settable(L, -3);
	}
	lua_settable(L, -3);
	return 0;
}

static int lua_opendb(lua_State *L){
	const char *file = luaL_checkstring(L, 1);
    int mode = luaL_optinteger(L, 2, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
	sqlite3 **db = (sqlite3 **)lua_newuserdata(L, sizeof(sqlite3 *));
	if (sqlite3_open_v2(file, db, mode, NULL) != SQLITE_OK){
        sqlite3_close(db);
		return luaL_error(L, sqlite3_errmsg(db));
	}
	luaL_getmetatable(L, "sqlite3");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_closedb(lua_State *L){
	sqlite3 *db = *(sqlite3 **)luaL_checkudata(L, 1, "sqlite3");
	sqlite3_close(db);
	return 0;
}

static int lua_query(lua_State *L){
	sqlite3 *db = *(sqlite3 **)luaL_checkudata(L, 1, "sqlite3");
	const char *query = luaL_checkstring(L, 2);
	callback_results = 1;
	lua_newtable(L);
	int fd = sqlite3_exec(db, query, sqlite_callback, L, NULL);
	if (fd != SQLITE_OK) {
		return luaL_error(L, sqlite3_errmsg(db));
	}
	return 1;
}

static const struct luaL_Reg sqlite3_lib[] = {
    {"open", lua_opendb},
    {"query", lua_query},
    {"close", lua_closedb},
    {NULL, NULL}
};

static const struct luaL_Reg sqlite3_methods[] = {
    {"query", lua_query},
    {"close", lua_closedb},
    {"__gc", lua_closedb},
    {NULL, NULL}
};

void luaL_opensqlite3(lua_State *L) {
	luaL_newmetatable(L, "sqlite3");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_openlib(L, NULL, sqlite3_methods, 0);

	luaL_openlib(L, "sqlite3", sqlite3_lib, 0);
    luaL_pushglobalint(L, SQLITE_OPEN_READONLY);
    luaL_pushglobalint(L, SQLITE_OPEN_READWRITE);
    luaL_pushglobalint(L, SQLITE_OPEN_CREATE);
    luaL_pushglobalint(L, SQLITE_OPEN_URI);
    luaL_pushglobalint(L, SQLITE_OPEN_NOMUTEX);
    luaL_pushglobalint(L, SQLITE_OPEN_FULLMUTEX);
}