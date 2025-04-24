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

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

	L = luaL_newstate();
	luaL_openlibs(L);

	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		psvDebugScreenInit();
		printf("LifeLua has encountered an error:\n%s", lua_tostring(L, -1));
		sceKernelDelayThread(10*1000000);
	}

	lua_close(L);
	sceKernelExitProcess(0);
	return 0;
}