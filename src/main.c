#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include <vitasdk.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#include "debugScreen.h"
#define printf psvDebugScreenPrintf

lua_State *L;

int lua_delay(lua_State *L) {
	int argc = lua_gettop(L);
	int secs;
	if(argc<=0){
		secs = 0;
	}
	else{
		secs = luaL_checkinteger(L, 1);
	}
    sceKernelDelayThread(secs * 1000000); // this converts microsecs to secs
    return 0;
}
int lua_exit(lua_State *L) {
    sceKernelExitProcess(0);
    return 0;
}

// "extending" the "os" module
static const struct luaL_Reg os_lib[] = {
    {"delay", lua_delay},
    {"exit", lua_exit},
    {NULL, NULL}
};

void luaL_openos(lua_State *L) {
	lua_getglobal(L, "os"); // Get existing os table
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);        // Remove whatever was on the stack
		lua_newtable(L);      // Create a new one if it doesn’t exist
		lua_setglobal(L, "os");
		lua_getglobal(L, "os");
	}

	luaL_setfuncs(L, os_lib, 0); // Add your functions to the os table
	lua_pop(L, 1); // Pop the os table off the stack
}

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_openos(L);

	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		psvDebugScreenInit();
		printf("LifeLua has encountered an error:\n%s", lua_tostring(L, -1));
		sceKernelDelayThread(10*1000000);
	}

	lua_close(L);
	sceKernelExitProcess(0);
	return 0;
}