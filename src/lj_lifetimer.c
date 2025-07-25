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
#include "include/libtimer.h"

#include "lj_lifeinit.h"

typedef struct {
    Timer *timer;
} lTimer;

static int lua_newtimer(lua_State *L){
    lTimer *timer = (lTimer *)lua_newuserdata(L, sizeof(lTimer));

    timer->timer = createTimer();
    
    luaL_getmetatable(L, "timer");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_timerstart(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    startTimer(timer->timer);
    return 0;
}

static int lua_timerpause(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    pauseTimer(timer->timer);
    return 0;
}

static int lua_timerreset(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    resetTimer(timer->timer);
    return 0;
}

static int lua_timerstop(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    stopTimer(timer->timer);
    return 0;
}

static int lua_timerelapsed(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    if (timer->timer->running) updateTimer(timer->timer);
	lua_pushnumber(L, (double)(timer->timer->elapsed)/(1000*1000));
    return 1;
}

static int lua_timerrunning(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    lua_pushboolean(L, timer->timer->running);
    return 1;
}

static int lua_timerset(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    double seconds = luaL_checknumber(L, 2);
    SceRtcTick rtick;
	sceRtcGetCurrentTick(&rtick);

	uint64_t offset_ticks = (uint64_t)(seconds * 1000000.0); // seconds to microseconds
	timer->timer->start_time = rtick.tick - offset_ticks;
	timer->timer->current_time = rtick.tick;
	timer->timer->elapsed = offset_ticks;
	timer->timer->running = true;
    return 0;
}

static int lua_timergc(lua_State *L){
    lTimer *timer = (lTimer *)luaL_checkudata(L, 1, "timer");
    freeTimer(timer->timer);
    return 1;
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
    {"__gc", lua_timergc},
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