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
#include <vita2d.h>

#include "lj_lifeinit.h"

typedef struct {
	SceUInt64 elapsed;
	bool running;
	SceUInt64 current_time;
	SceUInt64 start_time;
	SceUInt64 pause_time;
	SceUInt64 pause_start;
	SceUInt64 pause_end;
} Timer;

void updateTimer(Timer *timer){	
	if(!timer->running) return;
	
	timer->current_time = sceKernelGetProcessTimeWide();
	
	timer->elapsed = timer->current_time - timer->start_time;
}

static int lua_newtimer(lua_State *L){
    Timer *timer = (Timer *)lua_newuserdata(L, sizeof(Timer));

	timer->start_time = sceKernelGetProcessTimeWide();
    
    luaL_getmetatable(L, "timer");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_timerstart(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
	
	timer->start_time = sceKernelGetProcessTimeWide();
	timer->current_time = 0;
	timer->running = true;
    return 0;
}

static int lua_timerpause(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
	
	if(timer->running) timer->pause_start = sceKernelGetProcessTimeWide();
	else {
		timer->pause_end = sceKernelGetProcessTimeWide();
		timer->pause_time = timer->pause_end - timer->pause_start;
		timer->start_time += timer->pause_time;
	}
	
	timer->running = !timer->running;
    return 0;
}

static int lua_timerreset(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    //this is how to reset a timer
	timer->elapsed = 0;
	timer->running = false;
	timer->current_time = 0;
	timer->pause_end = 0;
	timer->pause_start = 0;
	timer->pause_time = 0;
	timer->start_time = 0;
    return 0;
}

static int lua_timerstop(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    timer->running = false;
    return 0;
}

static int lua_timerelapsed(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    if (timer->running) updateTimer(timer);
	lua_pushnumber(L, (double)(timer->elapsed)/(1000*1000));
    return 1;
}

static int lua_timerrunning(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    lua_pushboolean(L, timer->running);
    return 1;
}

static int lua_timerset(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    double seconds = luaL_checknumber(L, 2);

	SceUInt64 offset_ticks = (SceUInt64)(seconds * 1000000); // seconds to microseconds
	timer->start_time = sceKernelGetProcessTimeWide() - offset_ticks;
	timer->current_time = sceKernelGetProcessTimeWide();
	timer->elapsed = offset_ticks;
	timer->running = true;
    return 0;
}

static const luaL_Reg timer_lib[] = {
    {"new", lua_newtimer},
    {"start", lua_timerstart},
    {"pause", lua_timerpause},
    {"stop", lua_timerstop},
    {"reset", lua_timerreset},
    {"set", lua_timerset},
    {"elapsed", lua_timerelapsed},
    {"running", lua_timerrunning},
    {NULL, NULL}
};

static const luaL_Reg timer_methods[] = {
    {"start", lua_timerstart},
    {"pause", lua_timerpause},
    {"stop", lua_timerstop},
    {"reset", lua_timerreset},
    {"set", lua_timerset},
    {"elapsed", lua_timerelapsed},
    {"running", lua_timerrunning},
    {NULL, NULL}
};

LUALIB_API int luaL_opentimer(lua_State *L) {
	luaL_newmetatable(L, "timer");

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, timer_methods);

	luaL_register(L, "timer", timer_lib);
    return 1;
}