/*
    LifeLua WIP
    Timer library
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
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/motion.h>
#include <psp2/kernel/processmgr.h>
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

typedef struct {
    SceUInt64 start_time;
    SceUInt64 stop_time;
    SceUInt64 pause_time;
    SceUInt64 total_paused;
    int running;
    int paused;
} Timer;

static int timer_new(lua_State *L) {
    Timer *t = (Timer *)lua_newuserdata(L, sizeof(Timer));
    t->start_time = 0;
    t->stop_time = 0;
    t->running = 0;

    luaL_getmetatable(L, "timer");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_starttimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    t->start_time = sceKernelGetProcessTimeWide();
    t->stop_time = 0;
    t->pause_time = 0;
    t->total_paused = 0;
    t->running = 1;
    t->paused = 0;
    return 0;
}

static int lua_stoptimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && !t->paused) {
        t->stop_time = sceKernelGetProcessTimeWide();
    }
    t->running = 0;
    t->paused = 0;
    return 0;
}

static int lua_pausetimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && !t->paused) {
        t->pause_time = sceKernelGetProcessTimeWide();
        t->paused = 1;
    }
    return 0;
}

static int lua_resumetimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && t->paused) {
        SceUInt64 now = sceKernelGetProcessTimeWide();
        t->total_paused += now - t->pause_time;
        t->paused = 0;
    }
    return 0;
}

static int lua_timerelapsed(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    SceUInt64 now;
    if (t->paused)
        now = t->pause_time;
    else if (!t->running)
        now = t->stop_time;
    else
        now = sceKernelGetProcessTimeWide();

    double elapsed = (double)(now - t->start_time - t->total_paused) / 1000000.0;
    lua_pushnumber(L, elapsed);
    return 1;
}

static int lua_settimer(lua_State *L){
	Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
	SceUInt64 starting_point = luaL_optnumber(L, 2, 0);
	t->start_time = starting_point;
	return 0;
}

static int lua_resettimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
	SceUInt64 starting_point = luaL_optnumber(L, 2, 0);
    t->start_time = starting_point;
    t->stop_time = 0;
    t->pause_time = 0;
    t->total_paused = 0;
    t->running = 0;
    t->paused = 0;
    return 0;
}

static int lua_istimerrunning(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    lua_pushboolean(L, t->running && !(t->paused));
    return 1;
}

static const luaL_Reg timer_methods[] = {
    {"start", lua_starttimer},
    {"stop", lua_stoptimer},
    {"pause", lua_pausetimer},
    {"resume", lua_resumetimer},
    {"elapsed", lua_timerelapsed},
    {"reset", lua_resettimer},
	{"set", lua_settimer},
	{"isrunning", lua_istimerrunning},
    {NULL, NULL}
};

static const luaL_Reg timer_lib[] = {
    {"new", timer_new},
    {NULL, NULL}
};

void luaL_opentimer(lua_State *L) {
	luaL_newmetatable(L, "timer");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, timer_methods, 0);
    lua_pop(L, 1);

	lua_newtable(L);
	luaL_setfuncs(L, timer_lib, 0);
	lua_setglobal(L, "timer");
}