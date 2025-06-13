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
#include "include/ftpvita.h"
#include "include/sha1.h"

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#define FRAMERATES_NUM 9 // Number of available framerates

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
	vita2d_free_texture(camerabuf->tex);
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

static const struct luaL_Reg camera_lib[] = {
    {"start", lua_start},
    {"output", lua_output},
    {"reverse", lua_reverse},
    {"effect", lua_effect},
    {"zoom", lua_zoom},
    {"stop", lua_stop},
    {NULL, NULL}
};

void luaL_opencamera(lua_State *L){
    luaL_openlib(L, "camera", camera_lib, 0);
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
}