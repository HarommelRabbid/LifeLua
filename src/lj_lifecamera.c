/*
    LifeLua WIP
    Camera library
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

SceCameraInfo cam_info;
SceCameraRead cam_info_read;
Image *camerabuf;
SceCameraDevice cam_type;
int camerabuf_ref = LUA_NOREF;
bool camera = false;

void initCam(lua_State *L, SceCameraDevice type, SceCameraResolution res, SceCameraFrameRate fps){
	uint16_t width;
	uint16_t height;
	bool high_fps_ready = false;
    switch (res){
        case SCE_CAMERA_RESOLUTION_0_0: //invalid
	    	width = 0;
	    	height = 0;
            break;
	    case SCE_CAMERA_RESOLUTION_640_480: //vga
	    	width = 640;
	    	height = 480;
            break;
	    case SCE_CAMERA_RESOLUTION_320_240: //qvga
	    	width = 320;
	    	height = 240;
	    	high_fps_ready = true;
            break;
	    case SCE_CAMERA_RESOLUTION_160_120: //qqvga
	    	width = 160;
	    	height = 120;
	    	high_fps_ready = true;
            break;
	    case SCE_CAMERA_RESOLUTION_352_288: //cif
	    	width = 352;
	    	height = 288;
	    	high_fps_ready = true;
            break;
	    case SCE_CAMERA_RESOLUTION_176_144: //qcif
	    	width = 176;
	    	height = 144;
            break;
	    case SCE_CAMERA_RESOLUTION_480_272: //psp
	    	width = 480;
	    	height = 272;
            break;
	    case SCE_CAMERA_RESOLUTION_640_360: //ngp
	    	width = 640;
	    	height = 360;
            break;
        default:
            width = 0;
            height = 0;
            break;
	}
	
	// Initializing camera buffers
	SceKernelMemBlockType orig = vita2d_texture_get_alloc_memblock_type();
	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW);
	camerabuf = (Image *)lua_newuserdata(L, sizeof(Image));
    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
	camerabuf->tex = vita2d_create_empty_texture(width, height);
	vita2d_texture_set_alloc_memblock_type(orig);
    camerabuf_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	
	// Initializing camera structs
	cam_info.size = sizeof(SceCameraInfo);
	cam_info.format = SCE_CAMERA_FORMAT_ABGR;
	cam_info.resolution = res;
	cam_info.pitch = vita2d_texture_get_stride(camerabuf->tex) - (width << 2);
	cam_info.sizeIBase = (width * height) << 2;
	cam_info.pIBase = vita2d_texture_get_datap(camerabuf->tex);
	
	cam_info_read.size = sizeof(SceCameraRead);
	cam_info_read.mode = 0;
	
	// Setting framerate
	if ((fps == 120) && (!high_fps_ready)) cam_info.framerate = 60;
	else cam_info.framerate = fps;
	
	sceCameraOpen(type, &cam_info);
	sceCameraStart(type);
    cam_type = type;
}

static int lua_start(lua_State *L){
    if (camera) return luaL_error(L, "Camera has already been started");
    SceCameraDevice type = luaL_checkinteger(L, 1);
    SceCameraResolution res = luaL_checkinteger(L, 2);
    SceCameraFrameRate fps = luaL_optinteger(L, 3, SCE_CAMERA_FRAMERATE_60_FPS);
    initCam(L, type, res, fps);
    camera = true;
    return 0;
}

static int lua_output(lua_State *L){
	if (!camera) return luaL_error(L, "Camera wasn't started");
	sceCameraRead(cam_type, &cam_info_read);
    if (camerabuf_ref == LUA_NOREF) return luaL_error(L, "Camera buffer not initialized");
    lua_rawgeti(L, LUA_REGISTRYINDEX, camerabuf_ref); // push image userdata
	return 1;
}

static int lua_stop(lua_State *L){
	if (!camera) return luaL_error(L, "Camera wasn't started");
	camera = false;
	sceCameraStop(cam_type);
	sceCameraClose(cam_type);
	//vita2d_free_texture(camerabuf->tex);
	return 0;
}

static int lua_reverse(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
        SceCameraReverse mode = luaL_checkinteger(L, 1);
        sceCameraSetReverse(cam_type, mode);
        return 0;
    }else{
        int val;
	    sceCameraGetReverse(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_effect(lua_State *L){
	if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraEffect effect = luaL_checkinteger(L, 1);
	    sceCameraSetEffect(cam_type, effect);
	    return 0;
    }else{
        int val;
	    sceCameraGetEffect(cam_type, &val);
	    lua_pushnumber(L, val);
    }
    return 1;
}

static int lua_zoom(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    int level = luaL_checkinteger(L, 1);
	    sceCameraSetZoom(cam_type, level);
	    return 0;
    }else{
        int val;
	    sceCameraGetZoom(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_brightness(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    int level = luaL_checkinteger(L, 1);
	    sceCameraSetBrightness(cam_type, CLAMP(level, 0, 255));
	    return 0;
    }else{
        int val;
	    sceCameraGetBrightness(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_saturation(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraSaturation level = luaL_checkinteger(L, 1);
	    sceCameraSetSaturation(cam_type, level);
	    return 0;
    }else{
        int val;
	    sceCameraGetSaturation(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_sharpness(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraSharpness level = luaL_checkinteger(L, 1);
	    sceCameraSetSharpness(cam_type, level);
	    return 0;
    }else{
        int val;
	    sceCameraGetSharpness(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_contrast(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    int level = luaL_checkinteger(L, 1);
	    sceCameraSetContrast(cam_type, CLAMP(level, 0, 255));
	    return 0;
    }else{
        int val;
	    sceCameraGetContrast(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_ev(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraExposureCompensation level = luaL_checkinteger(L, 1);
	    sceCameraSetEV(cam_type, level);
	    return 0;
    }else{
        int val;
	    sceCameraGetEV(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_antiflicker(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraAntiFlicker mode = luaL_checkinteger(L, 1);
	    sceCameraSetAntiFlicker(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetAntiFlicker(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_iso(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraISO mode = luaL_checkinteger(L, 1);
	    sceCameraSetISO(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetISO(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_gain(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraGain mode = luaL_checkinteger(L, 1);
	    sceCameraSetGain(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetGain(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_whitebalance(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraWhiteBalance mode = luaL_checkinteger(L, 1);
	    sceCameraSetWhiteBalance(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetWhiteBalance(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_backlight(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraBacklight mode = luaL_checkinteger(L, 1);
	    sceCameraSetBacklight(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetBacklight(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_nightmode(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
	    SceCameraNightmode mode = luaL_checkinteger(L, 1);
	    sceCameraSetNightmode(cam_type, mode);
	    return 0;
    }else{
        int val;
	    sceCameraGetNightmode(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_noisereduction(lua_State *L){
    if (!camera) return luaL_error(L, "Camera wasn't started");
    if(!lua_isnone(L, 1)){
        int level = luaL_checkinteger(L, 1);
        sceCameraSetNoiseReduction(cam_type, level);
        return 0;
    }else{
        int val;
	    sceCameraGetNoiseReduction(cam_type, &val);
	    lua_pushinteger(L, val);
    }
    return 1;
}

static int lua_active(lua_State *L){
    lua_pushboolean(L, sceCameraIsActive(cam_type));
    return 1;
}

static const luaL_Reg camera_lib[] = {
    {"start", lua_start},
    {"output", lua_output},
    {"reverse", lua_reverse},
    {"effect", lua_effect},
    {"zoom", lua_zoom},
    {"brightness", lua_brightness},
    {"saturation", lua_saturation},
    {"sharpness", lua_sharpness},
    {"contrast", lua_contrast},
    {"exposure", lua_ev},
    {"antiflicker", lua_antiflicker},
    {"iso", lua_iso},
    {"gain", lua_gain},
    {"whitebalance", lua_whitebalance},
    {"backlight", lua_backlight},
    {"nightmode", lua_nightmode},
    {"noisereduction", lua_noisereduction},
    {"active", lua_active},
    {"stop", lua_stop},
    {NULL, NULL}
};

LUALIB_API int luaL_opencamera(lua_State *L){
    luaL_register(L, "camera", camera_lib);
    luaL_pushglobalint(L, SCE_CAMERA_DEVICE_FRONT);
    luaL_pushglobalint(L, SCE_CAMERA_DEVICE_BACK);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_0_0);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_160_120);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_176_144);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_320_240);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_352_288);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_480_272);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_640_360);
    luaL_pushglobalint(L, SCE_CAMERA_RESOLUTION_640_480);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_10_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_120_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_15_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_20_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_30_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_3_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_5_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_60_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_FRAMERATE_7_FPS);
    luaL_pushglobalint(L, SCE_CAMERA_REVERSE_OFF);
    luaL_pushglobalint(L, SCE_CAMERA_REVERSE_FLIP);
    luaL_pushglobalint(L, SCE_CAMERA_REVERSE_MIRROR);
    luaL_pushglobalint(L, SCE_CAMERA_REVERSE_MIRROR_FLIP);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_NORMAL);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_NEGATIVE);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_BLACKWHITE);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_SEPIA);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_BLUE);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_RED);
	luaL_pushglobalint(L, SCE_CAMERA_EFFECT_GREEN);
    luaL_pushglobalint(L, SCE_CAMERA_SATURATION_0);
    luaL_pushglobalint(L, SCE_CAMERA_SATURATION_5);
	luaL_pushglobalint(L, SCE_CAMERA_SATURATION_10);
	luaL_pushglobalint(L, SCE_CAMERA_SATURATION_20);
	luaL_pushglobalint(L, SCE_CAMERA_SATURATION_30);
	luaL_pushglobalint(L, SCE_CAMERA_SATURATION_40);
    luaL_pushglobalint(L, SCE_CAMERA_SHARPNESS_100);
	luaL_pushglobalint(L, SCE_CAMERA_SHARPNESS_200);
	luaL_pushglobalint(L, SCE_CAMERA_SHARPNESS_300);
	luaL_pushglobalint(L, SCE_CAMERA_SHARPNESS_400);
    luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_20);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_17);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_15);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_13);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_10);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_7);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_5);
	luaL_pushglobalint(L, SCE_CAMERA_EV_NEGATIVE_3);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_0);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_3);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_5);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_7);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_10);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_13);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_15);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_17);
	luaL_pushglobalint(L, SCE_CAMERA_EV_POSITIVE_20);
    luaL_pushglobalint(L, SCE_CAMERA_ANTIFLICKER_AUTO);
	luaL_pushglobalint(L, SCE_CAMERA_ANTIFLICKER_50HZ);
	luaL_pushglobalint(L, SCE_CAMERA_ANTIFLICKER_60HZ);
    luaL_pushglobalint(L, SCE_CAMERA_ISO_AUTO);
	luaL_pushglobalint(L, SCE_CAMERA_ISO_100);
	luaL_pushglobalint(L, SCE_CAMERA_ISO_200);
	luaL_pushglobalint(L, SCE_CAMERA_ISO_400);
    luaL_pushglobalint(L, SCE_CAMERA_GAIN_AUTO);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_1);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_2);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_3);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_4);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_5);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_6);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_7);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_8);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_9);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_10);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_11);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_12);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_13);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_14);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_15);
	luaL_pushglobalint(L, SCE_CAMERA_GAIN_16);
    luaL_pushglobalint(L, SCE_CAMERA_WB_AUTO);
	luaL_pushglobalint(L, SCE_CAMERA_WB_DAY);
	luaL_pushglobalint(L, SCE_CAMERA_WB_CWF);
	luaL_pushglobalint(L, SCE_CAMERA_WB_SLSA);
    luaL_pushglobalint(L, SCE_CAMERA_BACKLIGHT_OFF);
	luaL_pushglobalint(L, SCE_CAMERA_BACKLIGHT_ON);
    luaL_pushglobalint(L, SCE_CAMERA_NIGHTMODE_OFF);
	luaL_pushglobalint(L, SCE_CAMERA_NIGHTMODE_LESS10);
	luaL_pushglobalint(L, SCE_CAMERA_NIGHTMODE_LESS100);
	luaL_pushglobalint(L, SCE_CAMERA_NIGHTMODE_OVER100);
    return 1;
}