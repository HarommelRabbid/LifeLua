/*
    LifeLua WIP
    Thread library
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

typedef struct {
    SceUID thread;
    int func_ref;      // Lua registry reference to the function
    lua_State *L;      // Main Lua state (for reference)
} Thread;

static int lua_thread(SceSize args, void *argp) {
    Thread *thread_data = (Thread *)argp;

    // Create a new coroutine from the main Lua state
    lua_State *T = lua_newthread(thread_data->L);

    // Push the function to the new thread's stack
    lua_rawgeti(T, LUA_REGISTRYINDEX, thread_data->func_ref);

    lua_pcall(T, 0, 0, 0);

    luaL_unref(thread_data->L, LUA_REGISTRYINDEX, thread_data->func_ref);
    sceKernelExitDeleteThread(0);
    return 0;
}

static int lua_new(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const char *name = luaL_optstring(L, 2, "LifeLua Thread");
    Thread *thread = (Thread *)lua_newuserdata(L, sizeof(Thread));
    //memset(thread, 0, sizeof(Thread));
    lua_pushvalue(L, 1);
    thread->func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    thread->L = L;
    thread->thread = sceKernelCreateThread(name, &lua_thread, 0x10000100, 0x100000, 0, 0, NULL);
    luaL_getmetatable(L, "thread");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_start(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    int res = sceKernelStartThread(thread->thread, sizeof(Thread), thread);
    if (res < 0) {
        return luaL_error(L, "Failed to start thread (err: 0x%08X)", res);
    }
    return 0;
}

static int lua_exit(lua_State *L){
    sceKernelExitThread(0);
    return 0;
}

static int lua_exitdelete(lua_State *L){
    sceKernelExitDeleteThread(0);
    return 0;
}

static int lua_wait(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    float secs = luaL_optnumber(L, 2, 0);
    sceKernelWaitThreadEnd(thread->thread, NULL, NULL);
    return 0;
}

static int lua_waitcb(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    float secs = luaL_optnumber(L, 2, 0);
    sceKernelWaitThreadEndCB(thread->thread, NULL, NULL);
    return 0;
}

static const luaL_Reg thread_lib[] = {
    {"new", lua_new},
    {"start", lua_start},
    {"wait", lua_wait},
    {"waitcb", lua_waitcb},
    {"exit", lua_exit},
    {"exitdelete", lua_exitdelete},
    {NULL, NULL}
};

static const luaL_Reg thread_methods[] = {
    {"start", lua_start},
    {"wait", lua_wait},
    {"waitcb", lua_waitcb},
    {NULL, NULL}
};

LUALIB_API int luaL_openthread(lua_State *L){
    luaL_newmetatable(L, "thread");
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, thread_methods);
    luaL_register(L, "thread", thread_lib);
    return 1;
}