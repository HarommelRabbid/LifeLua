#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>

#include <vitasdk.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

lua_State *L;
vita2d_pgf *pgf;
vita2d_pvf *pvf;

int lua_delay(lua_State *L) {
	int secs = luaL_optinteger(L, 1, 0);
    sceKernelDelayThread(secs * 1000000); // this converts microsecs to secs
    return 0;
}
int lua_exit(lua_State *L) {
    sceKernelExitProcess(0);
    return 0;
}

static const struct luaL_Reg os_lib[] = {
    {"delay", lua_delay},
    {"exit", lua_exit},
    {NULL, NULL}
};

void luaL_extendos(lua_State *L) {
	lua_getglobal(L, "os"); // Get existing os table
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setglobal(L, "os");
		lua_getglobal(L, "os");
	}

	luaL_setfuncs(L, os_lib, 0); // extending the os library
	lua_pop(L, 1);
}

int lua_range(lua_State *L) {
	double num = luaL_checknumber(L, 1);
	double lower = luaL_checknumber(L, 2);
	double upper = luaL_checknumber(L, 3);
	lua_pushnumber(L, fmin(upper, fmax(num, lower)));
	return 1;
}

static const struct luaL_Reg math_lib[] = {
    {"range", lua_range},
    {NULL, NULL}
};

void luaL_extendmath(lua_State *L) {
	lua_getglobal(L, "math");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setglobal(L, "math");
		lua_getglobal(L, "math");
	}

	luaL_setfuncs(L, math_lib, 0); // extending the math library
	lua_pop(L, 1);
}

int lua_text(lua_State *L){
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	const char* text = (const char*)luaL_checkstring(L, 3);
	unsigned int color = luaL_checkinteger(L, 4);
	float size = luaL_optnumber(L, 5, 1.0f);

    vita2d_pgf_draw_text(pgf, x, y+20 * size, color, size, text);
	return 0;
}

// Draw rectangle
int lua_rect(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    unsigned int color = luaL_checkinteger(L, 5);

    vita2d_draw_rectangle(x, y, width, height, color);
    return 0;
}

// Draw circle
int lua_circle(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    unsigned int color = luaL_checkinteger(L, 4);

    vita2d_draw_fill_circle(x, y, radius, color);
    return 0;
}

// Draw line
int lua_line(lua_State *L) {
    int x0 = luaL_checkinteger(L, 1);
    int y0 = luaL_checkinteger(L, 2);
    int x1 = luaL_checkinteger(L, 3);
    int y1 = luaL_checkinteger(L, 4);
    unsigned int color = luaL_checkinteger(L, 5);

    vita2d_draw_line(x0, y0, x1, y1, color);
    return 0;
}

int lua_swapbuff(lua_State *L) {
	int color = luaL_optinteger(L, 1, RGBA8(0, 0, 0, 255));
    vita2d_end_drawing();
    vita2d_swap_buffers();
    vita2d_start_drawing();
	vita2d_set_clear_color(color);
    vita2d_clear_screen(); // Clear for next frame
    return 0;
}


static const struct luaL_Reg draw_lib[] = {
    {"text", lua_text},
    {"rect", lua_rect},
    {"circle", lua_circle},
    {"line", lua_line},
    {"swapbuffers", lua_swapbuff},
    {NULL, NULL}
};

void luaL_opendraw(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, draw_lib, 0);
	lua_setglobal(L, "draw");
}

int lua_newcolor(lua_State *L) {
	int r = luaL_checkinteger(L, 1);
	int g = luaL_checkinteger(L, 2);
	int b = luaL_checkinteger(L, 3);
	int a = luaL_optinteger(L, 4, 255);
	lua_pushinteger(L, RGBA8(r, g, b, a));
	return 1;
}

static const struct luaL_Reg color_lib[] = {
    {"new", lua_newcolor},
    {NULL, NULL}
};

void luaL_opencolor(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, color_lib, 0);
	lua_setglobal(L, "color");
}

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

	vita2d_init();
    vita2d_set_clear_color(RGBA8(0, 0, 0, 255));
	pgf = vita2d_load_default_pgf();
	pvf = vita2d_load_default_pvf();

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_extendos(L);
	luaL_extendmath(L);
	luaL_opendraw(L);
	luaL_opencolor(L);

	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		while(1){
			vita2d_start_drawing();
        	vita2d_clear_screen();
			vita2d_pvf_draw_text(pvf, 2, 20, RGBA8(255, 255, 255, 255), 1.0f, "LifeLua has encountered an error:");
			vita2d_pvf_draw_text(pvf, 2, 40, RGBA8(255, 255, 255, 255), 1.0f, lua_tostring(L, -1));
			vita2d_end_drawing();
    		vita2d_swap_buffers();
		}
	}

	lua_close(L);
	vita2d_fini();
	vita2d_free_pgf(pgf);
	vita2d_free_pvf(pvf);
	sceKernelExitProcess(0);
	return 0;
}