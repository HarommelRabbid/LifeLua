/*
    LifeLua WIP
    Controls library
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

static int lua_updatecontrols(lua_State *L){
	int argc = lua_gettop(L);
	bool ext = false;
	bool bind = false;
	if (argc >= 1) ext = lua_toboolean(L, 1);
	if (argc >= 2) bind = lua_toboolean(L, 2);
	oldpad = pad;
	if(!ext && !bind) sceCtrlPeekBufferPositive(0, &pad, 1);
	else if(ext && !bind) sceCtrlReadBufferPositiveExt(0, &pad, 1);
	else if(!ext && bind) sceCtrlReadBufferPositive2(0, &pad, 1);
	else if(ext && bind) sceCtrlReadBufferPositiveExt2(0, &pad, 1);
	sceMotionGetSensorState(&motion, 1);
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &fronttouch, 1);
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &reartouch, 1);
	return 0;
}

static int lua_check(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, pad.buttons & button);
	return 1;
}

static int lua_pressed(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, (pad.buttons & button) && !(oldpad.buttons & button));
	return 1;
}

static int lua_held(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, (pad.buttons & button) && (oldpad.buttons & button));
	return 1;
}

static int lua_released(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, !(pad.buttons & button) && (oldpad.buttons & button));
	return 1;
}
static int lua_analogl(lua_State *L){
	lua_pushinteger(L, pad.lx - 128);
	lua_pushinteger(L, pad.ly - 128);
	return 2;
}

static int lua_analogr(lua_State *L){
	lua_pushinteger(L, pad.rx - 128);
	lua_pushinteger(L, pad.ry - 128);
	return 2;
}

static int lua_accelerometer(lua_State *L){
	lua_newtable(L);

	lua_pushnumber(L, motion.accelerometer.x);
    lua_setfield(L, -2, "x");
	lua_pushnumber(L, motion.accelerometer.y);
    lua_setfield(L, -2, "y");
	lua_pushnumber(L, motion.accelerometer.z);
    lua_setfield(L, -2, "z");
	return 1;
}

static int lua_gyroscope(lua_State *L){
	lua_newtable(L);

	lua_pushnumber(L, motion.gyro.x);
    lua_setfield(L, -2, "x");
	lua_pushnumber(L, motion.gyro.y);
    lua_setfield(L, -2, "y");
	lua_pushnumber(L, motion.gyro.z);
    lua_setfield(L, -2, "z");
	return 1;
}

static int lua_fronttouch(lua_State *L){
	lua_newtable(L);

	for (int i = 0; i < fronttouch.reportNum; i++) {
		lua_newtable(L);

		lua_pushnumber(L, lerp(fronttouch.report[i].x, 1920, 960));
		lua_setfield(L, -2, "x");

		lua_pushnumber(L, lerp(fronttouch.report[i].y, 1315, 855));
		lua_setfield(L, -2, "y");

		lua_pushnumber(L, fronttouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushnumber(L, fronttouch.report[i].force);
        lua_setfield(L, -2, "force");

		lua_rawseti(L, -2, i+1);
	}
	return 1;
}

static int lua_reartouch(lua_State *L){
	lua_newtable(L);

	for (int i = 0; i < reartouch.reportNum; i++) {
		lua_newtable(L);

		lua_pushnumber(L, lerp(reartouch.report[i].x, 1920, 960));
		lua_setfield(L, -2, "x");

		lua_pushnumber(L, lerp(reartouch.report[i].y, 1285, 855));
		lua_setfield(L, -2, "y");

		lua_pushnumber(L, reartouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushnumber(L, reartouch.report[i].force);
        lua_setfield(L, -2, "force");

		lua_rawseti(L, -2, i+1);
	}
	return 1;
}

static int lua_actuator(lua_State *L){
	uint8_t small = luaL_checkinteger(L, 1);
	uint8_t large = luaL_checkinteger(L, 2);
    int port = luaL_optinteger(L, 3, 1);
	actuators[port-1].small = small;
	actuators[port-1].small = large;
	sceCtrlSetActuator(port, &actuators[port-1]);
	return 0;
}

static int lua_lightbar(lua_State *L){
	SceUInt8 r = luaL_checkinteger(L, 1);
    SceUInt8 g = luaL_checkinteger(L, 2);
    SceUInt8 b = luaL_checkinteger(L, 3);
    int port = luaL_optinteger(L, 4, 1);
	sceCtrlSetLightBar(port, r, g, b);
	return 0;
}

static const luaL_Reg controls_lib[] = {
	{"update", lua_updatecontrols},
	{"check", lua_check},
	{"pressed", lua_pressed},
	{"held", lua_held},
	{"released", lua_released},
	{"leftanalog", lua_analogl},
	{"rightanalog", lua_analogr},
	{"accelerometer", lua_accelerometer},
	{"gyroscope", lua_gyroscope},
	{"fronttouch", lua_fronttouch},
	{"reartouch", lua_reartouch},
    {"vibrate", lua_actuator},
    {"lightbar", lua_lightbar},
    {NULL, NULL}
};

LUALIB_API int luaL_opencontrols(lua_State *L) {
	luaL_register(L, "controls", controls_lib);
	luaL_pushglobalint(L, SCE_CTRL_UP);
	luaL_pushglobalint(L, SCE_CTRL_DOWN);
	luaL_pushglobalint(L, SCE_CTRL_LEFT);
	luaL_pushglobalint(L, SCE_CTRL_RIGHT);
	luaL_pushglobalint(L, SCE_CTRL_CROSS);
	luaL_pushglobalint(L, SCE_CTRL_CIRCLE);
	luaL_pushglobalint(L, SCE_CTRL_SQUARE);
	luaL_pushglobalint(L, SCE_CTRL_TRIANGLE);
	luaL_pushglobalint(L, SCE_CTRL_LTRIGGER);
	luaL_pushglobalint(L, SCE_CTRL_RTRIGGER);
	luaL_pushglobalint(L, SCE_CTRL_L1);
	luaL_pushglobalint(L, SCE_CTRL_R1);
	luaL_pushglobalint(L, SCE_CTRL_L2);
	luaL_pushglobalint(L, SCE_CTRL_R2);
	luaL_pushglobalint(L, SCE_CTRL_L3);
	luaL_pushglobalint(L, SCE_CTRL_R3);
	luaL_pushglobalint(L, SCE_CTRL_START);
	luaL_pushglobalint(L, SCE_CTRL_SELECT);
	luaL_pushglobalint(L, SCE_CTRL_POWER);
	luaL_pushglobalint(L, SCE_CTRL_VOLUP);
	luaL_pushglobalint(L, SCE_CTRL_VOLDOWN);
	luaL_pushglobalint(L, SCE_CTRL_PSBUTTON);
	luaL_pushglobalint(L, SCE_CTRL_INTERCEPTED);
	luaL_pushglobalint(L, SCE_CTRL_HEADPHONE);
	int enterButton;
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enterButton); 
	if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CROSS){
		luaL_pushglobalint_as(L, SCE_CTRL_CROSS, "SCE_CTRL_ACCEPT"); 
		luaL_pushglobalint_as(L, SCE_CTRL_CIRCLE, "SCE_CTRL_CANCEL");
	}
	else if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE){
		luaL_pushglobalint_as(L, SCE_CTRL_CIRCLE, "SCE_CTRL_ACCEPT"); 
		luaL_pushglobalint_as(L, SCE_CTRL_CROSS, "SCE_CTRL_CANCEL");
	}
    return 1;
}