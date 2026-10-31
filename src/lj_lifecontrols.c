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

static SceTouchPanelInfo rearinfo;

static int lua_updatecontrols(lua_State *L){
	int argc = lua_gettop(L);
	bool ext = lua_toboolean(L, 1);
	bool bind = lua_toboolean(L, 2);
	oldpad = pad;
	if(!ext && !bind) sceCtrlPeekBufferPositive(0, &pad, 1);
	else if(ext && !bind) sceCtrlPeekBufferPositiveExt(0, &pad, 1);
	else if(!ext && bind) sceCtrlPeekBufferPositive2(0, &pad, 1);
	else if(ext && bind) sceCtrlPeekBufferPositiveExt2(0, &pad, 1);
	sceMotionGetSensorState(&motion, 1);
	oldfronttouch = fronttouch; oldreartouch = reartouch;
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

static int lua_analogstick(lua_State *L){
	lua_newtable(L);

	lua_pushinteger(L, pad.lx - 128);
	lua_setfield(L, -2, "lx");
	lua_pushinteger(L, pad.ly - 128);
	lua_setfield(L, -2, "ly");
	lua_pushinteger(L, pad.rx - 128);
	lua_setfield(L, -2, "rx");
	lua_pushinteger(L, pad.ry - 128);
	lua_setfield(L, -2, "ry");
	return 1;
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
	int n = 1;

	for (int i = 0; i < fronttouch.reportNum; i++) {
		int was = 0;
		for (int j = 0; j < oldfronttouch.reportNum; j++)
			if (oldfronttouch.report[j].id == fronttouch.report[i].id) { was = 1; break; }

		lua_newtable(L);
		lua_pushnumber(L, fronttouch.report[i].x / 2);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, fronttouch.report[i].y / 2);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, fronttouch.report[i].id);
		lua_setfield(L, -2, "id");
		lua_pushnumber(L, fronttouch.report[i].force);
		lua_setfield(L, -2, "force");
		lua_pushboolean(L, !was);
		lua_setfield(L, -2, "pressed");
		lua_pushboolean(L, was);
		lua_setfield(L, -2, "held");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "released");
		lua_rawseti(L, -2, n++);
	}

	for (int i = 0; i < oldfronttouch.reportNum; i++) {
		int still = 0;
		for (int j = 0; j < fronttouch.reportNum; j++)
			if (fronttouch.report[j].id == oldfronttouch.report[i].id) { still = 1; break; }
		if (still) continue;

		lua_newtable(L);
		lua_pushnumber(L, oldfronttouch.report[i].x / 2);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, oldfronttouch.report[i].y / 2);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, oldfronttouch.report[i].id);
		lua_setfield(L, -2, "id");
		lua_pushnumber(L, oldfronttouch.report[i].force);
		lua_setfield(L, -2, "force");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "pressed");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "held");
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "released");
		lua_rawseti(L, -2, n++);
	}
	return 1;
}

static int lua_reartouch(lua_State *L){
	lua_newtable(L);
	int n = 1;

	float w = rearinfo.maxAaX - rearinfo.minAaX;
	float h = rearinfo.maxAaY - rearinfo.minAaY;

	for (int i = 0; i < reartouch.reportNum; i++) {
		int was = 0;
		for (int j = 0; j < oldreartouch.reportNum; j++)
			if (oldreartouch.report[j].id == reartouch.report[i].id) { was = 1; break; }

		lua_newtable(L);
		lua_pushnumber(L, (reartouch.report[i].x - rearinfo.minAaX) * 959.0f / w);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (reartouch.report[i].y - rearinfo.minAaY) * 543.0f / h);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, reartouch.report[i].id);
		lua_setfield(L, -2, "id");
		lua_pushnumber(L, reartouch.report[i].force);
		lua_setfield(L, -2, "force");
		lua_pushboolean(L, !was);
		lua_setfield(L, -2, "pressed");
		lua_pushboolean(L, was);
		lua_setfield(L, -2, "held");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "released");
		lua_rawseti(L, -2, n++);
	}

	for (int i = 0; i < oldreartouch.reportNum; i++) {
		int still = 0;
		for (int j = 0; j < reartouch.reportNum; j++)
			if (reartouch.report[j].id == oldreartouch.report[i].id) { still = 1; break; }
		if (still) continue;

		lua_newtable(L);
		lua_pushnumber(L, (reartouch.report[i].x - rearinfo.minAaX) * 959.0f / w);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (reartouch.report[i].y - rearinfo.minAaY) * 543.0f / h);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, oldreartouch.report[i].id);
		lua_setfield(L, -2, "id");
		lua_pushnumber(L, oldreartouch.report[i].force);
		lua_setfield(L, -2, "force");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "pressed");
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "held");
		lua_pushboolean(L, 1);
		lua_setfield(L, -2, "released");
		lua_rawseti(L, -2, n++);
	}
	return 1;
}

static int lua_actuator(lua_State *L){
	uint8_t small = luaL_checkinteger(L, 1);
	uint8_t large = luaL_checkinteger(L, 2);
    int port = luaL_optinteger(L, 3, 1);
	actuators[port-1].small = small;
	actuators[port-1].large = large;
	sceCtrlSetActuator(port, &actuators[port-1]);
	return 0;
}

static int lua_lightbar(lua_State *L){
	Color *col = (Color *)lua_tocolor(L, 1);
    int port = luaL_optinteger(L, 2, 1);
	sceCtrlSetLightBar(port, (col->color) & 0xFF, (col->color >> 8) & 0xFF, (col->color >> 16) & 0xFF);
	return 0;
}

static int controls_info(lua_State *L){
	SceCtrlPortInfo info;
	if (sceCtrlGetControllerPortInfo(&info) < 0) return 0;
	lua_newtable(L);
	for(int i = 0; i < 5; i++){
		lua_pushinteger(L, info.port[i]);
		lua_rawseti(L, -2, i+1);
	}
	return 1;
}

static const luaL_Reg controls_lib[] = {
	{"update", lua_updatecontrols},
	{"check", lua_check},
	{"pressed", lua_pressed},
	{"held", lua_held},
	{"released", lua_released},
	{"analogsticks", lua_analogstick},
	{"accelerometer", lua_accelerometer},
	{"gyroscope", lua_gyroscope},
	{"fronttouch", lua_fronttouch},
	{"reartouch", lua_reartouch},
    {"vibrate", lua_actuator},
    {"lightbar", lua_lightbar},
	{"info", controls_info},
    {NULL, NULL}
};

LUALIB_API int luaL_opencontrols(lua_State *L) {
	sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &rearinfo);
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
	luaL_pushglobalint(L, SCE_CTRL_TYPE_UNPAIRED);
	luaL_pushglobalint(L, SCE_CTRL_TYPE_PHY);
	luaL_pushglobalint(L, SCE_CTRL_TYPE_VIRT);
	luaL_pushglobalint(L, SCE_CTRL_TYPE_DS3);
	luaL_pushglobalint(L, SCE_CTRL_TYPE_DS4);
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