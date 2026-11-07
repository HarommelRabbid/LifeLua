/*
    LifeLua PS Vita
    Thread library
*/

#include "lj_lifeinit.h"

typedef struct {
    SceUID thread;
    char *script;
    size_t script_len;
    char *error;
    bool running;
} Thread;

static int lua_lifelua_thread(SceSize args, void *argp) {
    (void)args;
    Thread *thread_data = *(Thread **)argp;
    
    thread_data->running = true;

    lua_State *T = luaL_newstate();
    luaL_openlibs(T);
    luaL_openjson(T);
    luaL_openmp(T);
    luaL_opentoml(T);
    luaL_openutf8(T);
    luaL_opennetwork(T);
    luaL_opensocket(T);
    luaL_openclipboard(T);
    luaL_opencolor(T);
    luaL_extendio(T);
    luaL_extendos(T);
    luaL_opentimer(T);
    luaL_opensqlite3(T);
    luaL_openfont(T);
    luaL_opencontrols(T);
    luaL_extend(T);
    if((luaL_loadbuffer(T, thread_data->script, thread_data->script_len, "=LifeLua Thread") || lua_pcall(T, 0, 0, 0)) != LUA_OK){
        size_t err_len;
        const char *error = lua_tolstring(T, -1, &err_len);
        if(error != NULL){
            thread_data->error = malloc(err_len + 1);
            strcpy(thread_data->error, error);
        }else thread_data->error = strdup("Unknown error");
    }

    lua_close(T);

    thread_data->running = false;

    sceKernelExitDeleteThread(0);
    return 0;
}

static int thread_new(lua_State *L) {
    size_t script_len = 0;
    const char *script = luaL_checklstring(L, 1, &script_len);
    const char *name = luaL_optstring(L, 2, "LifeLua Thread");
    Thread *thread = (Thread *)lua_newuserdata(L, sizeof(Thread));
    thread->script = strndup(script, script_len);
    thread->script_len = script_len;
    thread->error = NULL;
    thread->thread = sceKernelCreateThread(name, &lua_lifelua_thread, 0x10000100, 0x100000, 0, 0, NULL);
    luaL_getmetatable(L, "thread");
    lua_setmetatable(L, -2);
    return 1;
}

static int thread_start(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    int res = sceKernelStartThread(thread->thread, sizeof(Thread *), &thread);
    if (res < 0) return luaL_error(L, "Failed to start thread (err: 0x%08X)", res);
    return 0;
}

static int thread_error(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    if(thread->error != NULL) lua_pushstring(L, thread->error);
    else lua_pushnil(L);
    return 1;
}

static int thread_running(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    lua_pushboolean(L, thread->running);
    return 1;
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
    sceKernelWaitThreadEnd(thread->thread, NULL, NULL);
    return 0;
}

static int lua_waitcb(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    sceKernelWaitThreadEndCB(thread->thread, NULL, NULL);
    return 0;
}

static int thread_gc(lua_State *L){
    Thread *thread = (Thread *)luaL_checkudata(L, 1, "thread");
    if(thread->error != NULL) free(thread->error);
    return 0;
}

static const luaL_Reg thread_lib[] = {
    {"new", thread_new},
    {"start", thread_start},
    {"error", thread_error},
    {"running", thread_running},
    {"wait", lua_wait},
    {"waitcb", lua_waitcb},
    {"exit", lua_exit},
    {"exitdelete", lua_exitdelete},
    {NULL, NULL}
};

static const luaL_Reg thread_methods[] = {
    {"start", thread_start},
    {"error", thread_error},
    {"running", thread_running},
    {"wait", lua_wait},
    {"waitcb", lua_waitcb},
    {"__gc", thread_gc},
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