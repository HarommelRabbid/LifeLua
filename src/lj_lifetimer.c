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
//#include "include/libtimer.h"

#include "lj_lifeinit.h"

typedef struct {
	uint32_t elapsed;
	bool running;
	uint32_t current_time;
	uint32_t start_time;
	uint32_t pause_time;
	uint32_t pause_start;
	uint32_t pause_end;
} Timer;

void updateTimer(Timer *timer){	
	if(!timer->running) return;
	
	SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->current_time = rtick.tick;
	
	timer->elapsed = timer->current_time - timer->start_time;
}

static int lua_newtimer(lua_State *L){
    Timer *timer = (Timer *)lua_newuserdata(L, sizeof(Timer));

    SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->start_time = rtick.tick;
    
    luaL_getmetatable(L, "timer");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_timerstart(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	timer->start_time = rtick.tick;
	timer->current_time = 0;
	timer->running = true;
    return 0;
}

static int lua_timerpause(lua_State *L){
    Timer *timer = (Timer *)luaL_checkudata(L, 1, "timer");
    SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);
	
	if(timer->running) timer->pause_start = rtick.tick;
	else {
		timer->pause_end = rtick.tick;
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
    SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);

	uint64_t offset_ticks = (uint64_t)(seconds * 1000000.0); // seconds to microseconds
	timer->start_time = rtick.tick - offset_ticks;
	timer->current_time = rtick.tick;
	timer->elapsed = offset_ticks;
	timer->running = true;
    return 0;
}

/*static int lua_timergc(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    freeTimer(timer->timer);
    return 1;
}*/

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
    //{"__gc", lua_timergc},
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