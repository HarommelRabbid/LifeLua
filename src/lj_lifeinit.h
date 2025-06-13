/*
	LifeLua WIP, a PS Vita LuaJIT interpreter
	by Harommel OddSock
*/

#ifndef LJ_LIFEINIT_H
#define LJ_LIFEINIT_H

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

#ifdef __cplusplus
#include <lua.hpp>
#else
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>
#endif
#define str(str) #str
#define luaL_pushglobalint(L, value) do { lua_pushinteger(L, value); lua_setglobal (L, str(value)); } while(0)
#define luaL_pushglobalint_as(L, value, var) do { lua_pushinteger(L, value); lua_setglobal (L, var); } while(0)
#define luaL_pushglobalint_alsoas(L, value, var) do { luaL_pushglobalint(L, value); luaL_pushglobalint_as(L, value, var); } while(0)
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#ifdef __cplusplus
extern "C" {
#endif

extern char vita_ip[16];
extern unsigned short int vita_port;

extern SceCtrlData pad, oldpad;
extern SceCtrlActuator actuators[4];
extern SceTouchData fronttouch, reartouch;
extern SceMotionSensorState motion;

extern bool unsafe;

extern vita2d_pgf *pgf;
extern vita2d_pvf *pvf;

typedef struct {
    unsigned int color;
} Color;

typedef struct {
	vita2d_pgf *pgf;
	vita2d_pvf *pvf;
	vita2d_font *font;
} Font;

typedef struct {
    vita2d_texture *tex;
} Image;

int string_ends_with(const char * str, const char * suffix);

int file_exists(const char* path);

void makeHeadBin(const char *dir);

void utf2ascii(char* dst, uint16_t* src);

void ascii2utf(uint16_t* dst, char* src);

void luaL_opentimer(lua_State *L);
void luaL_extendio(lua_State *L);
void luaL_extendos(lua_State *L);
void luaL_opennetwork(lua_State *L);
void luaL_opencontrols(lua_State *L);
void luaL_opensqlite3(lua_State *L);
void luaL_opendraw(lua_State *L);
void luaL_opencolor(lua_State *L);
void luaL_openimage(lua_State *L);
void luaL_openfont(lua_State *L);
void luaL_opencamera(lua_State *L);
//void luaL_openimgui(lua_State *L);
//void luaL_openjson(lua_State *L);

#ifdef __cplusplus
}
#endif
#endif