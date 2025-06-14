/*
    LifeLua WIP
    Image library
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

int lua_imageload(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
    
    if(file_exists(filename)){
		if(string_ends_with(filename, ".png")){
			image->tex = vita2d_load_PNG_file(filename);
		}else if((string_ends_with(filename, ".jpeg")) || (string_ends_with(filename, ".jpg"))){
			image->tex = vita2d_load_JPEG_file(filename);
		}else if(string_ends_with(filename, ".bmp")){
			image->tex = vita2d_load_BMP_file(filename);
		}else{
			lua_pushnil(L);
			//return luaL_error(L, "Image file type isn't accepted (must be a .png, .jpeg/.jpg, or a .bmp)");
		}
	}else{
		lua_pushnil(L);
	}
    if (!image->tex) /*return luaL_error(L, "Failed to load image: %s", filename)*/lua_pushnil(L);
    
    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_newimage(lua_State *L) {
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
	unsigned int w = luaL_checkinteger(L, 1);
	unsigned int h = luaL_checkinteger(L, 2);

	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW);

	image->tex = vita2d_create_empty_texture_rendertarget(w, h, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	if(lua_gettop(L) >= 3){
		Color *color = (Color *)luaL_checkudata(L, 3, "color");
		sceClibMemset(vita2d_texture_get_datap(image->tex), color->color, vita2d_texture_get_stride(image->tex) * h);
	}else sceClibMemset(vita2d_texture_get_datap(image->tex), 0xFFFFFFFF, vita2d_texture_get_stride(image->tex) * h);

    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_imagedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	Color *color;
	if(argc <= 3){
		vita2d_draw_texture(image->tex, x, y);
	}else{
		color = (Color *)luaL_checkudata(L, 4, "color");
		vita2d_draw_texture_tint(image->tex, x, y, color->color);
	}
	//vita2d_free_texture(image);
	return 0;
}

static int lua_imagescaledraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float scalex = luaL_checknumber(L, 4);
	float scaley = luaL_checknumber(L, 5);
	Color *color;
	if(argc <= 5){
		vita2d_draw_texture_scale(image->tex, x, y, scalex, scaley);
	}else{
		color = (Color *)luaL_checkudata(L, 6, "color");
		vita2d_draw_texture_tint_scale(image->tex, x, y, scalex, scaley, color->color);
	}
	//vita2d_free_texture(image);
	return 0;
}

static int lua_imagerotatedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float radius = luaL_checknumber(L, 4);
	Color *color;
	if(argc <= 5){
		vita2d_draw_texture_rotate(image->tex, x, y, radius);
	}else{
		color = (Color *)luaL_checkudata(L, 6, "color");
		vita2d_draw_texture_tint_rotate(image->tex, x, y, radius, color->color);
	}
	//vita2d_free_texture(image);
	return 0;
}

static int lua_imagewidth(lua_State *L){
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	lua_pushinteger(L, vita2d_texture_get_width(image->tex));
	return 1;
}

static int lua_imageheight(lua_State *L){
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	lua_pushinteger(L, vita2d_texture_get_height(image->tex));
	return 1;
}

int lua_imagegc(lua_State *L) {
    Image *image = (Image *)luaL_checkudata(L, 1, "image");
    if (image->tex) {
        vita2d_free_texture(image->tex);
    }
    return 0;
}

static const luaL_Reg image_lib[] = {
    {"load", lua_imageload},
	{"new", lua_newimage},
	//{"screen", lua_screenimage},
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"rotatedisplay", lua_imagerotatedraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
    {NULL, NULL}
};

static const luaL_Reg image_methods[] = {
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"rotatedisplay", lua_imagerotatedraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
	{"__gc", lua_imagegc},
    {NULL, NULL}
};

void luaL_openimage(lua_State *L) {
	luaL_newmetatable(L, "image");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_openlib(L, NULL, image_methods, 0);

	luaL_openlib(L, "image", image_lib, 0);
}