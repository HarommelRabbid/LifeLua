/*
	LifeLua WIP, a PS Vita LuaJIT interpreter
	by Harommel OddSock
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

lua_State *L;
vita2d_pgf *pgf;
vita2d_pvf *pvf;
vita2d_pvf *psexchar;
SceCtrlData pad, oldpad;
SceTouchData fronttouch, reartouch;
SceMotionSensorState motion;
SceCommonDialogConfigParam cmnDlgCfgParam;
bool unsafe = true;
char vita_ip[16];
unsigned short int vita_port = 0;

typedef struct {
    vita2d_texture *tex;
} Image;

typedef struct {
    unsigned int color;
} Color;

typedef struct {
	vita2d_pgf *pgf;
	vita2d_pvf *pvf;
	vita2d_font *font;
} Font;

int string_ends_with(const char * str, const char * suffix){
	int str_len = strlen(str);
	int suffix_len = strlen(suffix);

	return 
	(str_len >= suffix_len) &&
	(0 == strcmp(str + (str_len-suffix_len), suffix));
}

void utf2ascii(char* dst, uint16_t* src){
	if(!src || !dst)return;
	while(*src)*(dst++)=(*(src++))&0xFF;
	*dst=0x00;
}

void ascii2utf(uint16_t* dst, char* src){
	if(!src || !dst)return;
	while(*src)*(dst++)=(*src++);
	*dst=0x00;
}

int file_exists(const char* path) {
	SceIoStat stat;
	int res = sceIoGetstat(path, &stat);
	if(res >= 0) return 1;
	else return 0;
}

// lua functions
static int lua_text(lua_State *L){
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	const char *text = luaL_checkstring(L, 3);
	Color *color = (Color *)luaL_checkudata(L, 4, "color");
	float size;
	Font *font;

	if(lua_isuserdata(L, 5) && !lua_isnil(L, 5)){
		font = (Font *)luaL_checkudata(L, 5, "font");
		size = luaL_optnumber(L, 6, 1.0f);
		if(font->pgf != NULL) vita2d_pgf_draw_text(font->pgf, x, y+17.402 * size, color->color, size, text);
		else if(font->pvf != NULL) vita2d_pvf_draw_text(font->pvf, x, y+17.402 * size, color->color, size, text);
		else if(font->font != NULL) vita2d_font_draw_text(font->font, x, (y-6) + size*24, color->color, size*24, text);
	}else if(lua_isnumber(L, 5) && (lua_isuserdata(L, 6) && !lua_isnone(L, 6)) && !lua_isnil(L, 6)){
		font = (Font *)luaL_checkudata(L, 6, "font");
		size = luaL_optnumber(L, 5, 1.0f);
		if(font->pgf != NULL) vita2d_pgf_draw_text(font->pgf, x, y+17.402 * size, color->color, size, text);
		else if(font->pvf != NULL) vita2d_pvf_draw_text(font->pvf, x, y+17.402 * size, color->color, size, text);
		else if(font->font != NULL) vita2d_font_draw_text(font->font, x, (y-6) + size*24, color->color, size*24, text);
	}else if(lua_isnumber(L, 5) || lua_isnone(L, 5)){
		size = luaL_optnumber(L, 5, 1.0f);
		vita2d_pgf_draw_text(pgf, x, y+17.402 * size, color->color, size, text);
	}else{
		return luaL_typerror(L, 5, "number or font");
	}
	
	return 0;
}

// Draw rectangle
static int lua_rect(lua_State *L) {
	int argc = lua_gettop(L);
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    Color *color = (Color *)luaL_checkudata(L, 5, "color");
	Color *outline;
	if (argc == 6) outline = (Color *)luaL_checkudata(L, 6, "color");

    vita2d_draw_rectangle(x, y, width, height, color->color);
	if (argc == 6){
		vita2d_draw_line(x-1, y, x+width, y, outline->color);
		vita2d_draw_line(x, y, x, y+height-1, outline->color);
		vita2d_draw_line(width+x, y, width+x, y+height-1, outline->color);
		vita2d_draw_line(x-1, y+height, x+width, y+height, outline->color);
	}
    return 0;
}

// Draw circle
static int lua_circle(lua_State *L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    Color *color = (Color *)luaL_checkudata(L, 4, "color");

    vita2d_draw_fill_circle(x, y, radius, color->color);
    return 0;
}

// Draw line
static int lua_line(lua_State *L) {
    float x0 = luaL_checknumber(L, 1);
    float y0 = luaL_checknumber(L, 2);
    float x1 = luaL_checknumber(L, 3);
    float y1 = luaL_checknumber(L, 4);
    Color *color = (Color *)luaL_checkudata(L, 5, "color");

    vita2d_draw_line(x0, y0, x1, y1, color->color);
    return 0;
}

static int lua_swapbuff(lua_State *L) {
	Color *color;
	if (lua_gettop(L) >= 1) color = (Color *)luaL_checkudata(L, 5, "color");
    vita2d_end_drawing();
	vita2d_common_dialog_update();
	vita2d_wait_rendering_done();
    vita2d_swap_buffers();
    vita2d_start_drawing();
	if (lua_gettop(L) >= 1) vita2d_set_clear_color(color->color);
	else vita2d_set_clear_color(RGBA8(0, 0, 0, 255));
    vita2d_clear_screen(); // Clear for next frame
    return 0;
}

static int lua_textwidth(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	float size;
	Font *font;
	if(lua_isuserdata(L, 2) && !lua_isnil(L, 2)){
		font = (Font *)luaL_checkudata(L, 2, "font");
		size = luaL_optnumber(L, 3, 1.0f);
		if(font->pgf != NULL) lua_pushinteger(L, vita2d_pgf_text_width(font->pgf, size, text));
		else if(font->pvf != NULL) lua_pushinteger(L, vita2d_pvf_text_width(font->pvf, size, text));
		else if(font->font != NULL) lua_pushinteger(L, vita2d_font_text_width(font->font, size*24, text));
	}else if((lua_isnumber(L, 2) && (lua_isuserdata(L, 3) && !lua_isnone(L, 3))) && !lua_isnil(L, 3)){
		font = (Font *)luaL_checkudata(L, 3, "font");
		size = luaL_optnumber(L, 2, 1.0f);
		if(font->pgf != NULL) lua_pushinteger(L, vita2d_pgf_text_width(font->pgf, size, text));
		else if(font->pvf != NULL) lua_pushinteger(L, vita2d_pvf_text_width(font->pvf, size, text));
		else if(font->font != NULL) lua_pushinteger(L, vita2d_font_text_width(font->font, size*24, text));
	}else if(lua_isnumber(L, 2) || lua_isnone(L, 2)){
		size = luaL_optnumber(L, 2, 1.0f);
		lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
	}else{
		return luaL_typerror(L, 2, "number or font");
	}
	return 1;
}

static int lua_textheight(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	float size;
	Font *font;
	if(lua_isuserdata(L, 5)){
		font = (Font *)luaL_checkudata(L, 5, "font");
		size = luaL_optnumber(L, 6, 1.0f);
		if(font->pgf != NULL) lua_pushinteger(L, vita2d_pgf_text_height(font->pgf, size, text));
		else if(font->pvf != NULL) lua_pushinteger(L, vita2d_pvf_text_height(font->pvf, size, text));
		else if(font->font != NULL) lua_pushinteger(L, vita2d_font_text_height(font->font, size*20, text));
	}else if(lua_isnumber(L, 5) && (lua_isuserdata(L, 6) && !lua_isnone(L, 6))){
		font = (Font *)luaL_checkudata(L, 6, "font");
		size = luaL_optnumber(L, 5, 1.0f);
		if(font->pgf != NULL) lua_pushinteger(L, vita2d_pgf_text_height(font->pgf, size, text));
		else if(font->pvf != NULL) lua_pushinteger(L, vita2d_pvf_text_height(font->pvf, size, text));
		else if(font->font != NULL) lua_pushinteger(L, vita2d_font_text_height(font->font, size*20, text));
	}else if(lua_isnumber(L, 5) || lua_isnone(L, 5)){
		size = luaL_optnumber(L, 5, 1.0f);
		lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
	}else{
		return luaL_typerror(L, 5, "number or font");
	}
	return 1;
}

static int lua_pixel(lua_State *L){
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	Color *color = (Color *)luaL_checkudata(L, 3, "color");
	vita2d_draw_pixel(x, y, color->color);
	return 0;
}

static int lua_gradient(lua_State *L){
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    Color *top_left = (Color *)luaL_checkudata(L, 5, "color");
	Color *top_right = (Color *)luaL_checkudata(L, 6, "color");
	Color *bottom_left = (Color *)luaL_checkudata(L, 7, "color");
	Color *bottom_right = (Color *)luaL_checkudata(L, 8, "color");

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        6 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

    // Triangle 1: TL -> TR -> BL
    vertices[0] = (vita2d_color_vertex){x, y, 0.5f, top_left->color};
    vertices[1] = (vita2d_color_vertex){x + width, y, 0.5f, top_right->color};
    vertices[2] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left->color};

    // Triangle 2: TR -> BR -> BL
    vertices[3] = (vita2d_color_vertex){x + width, y, 0.5f, top_right->color};
    vertices[4] = (vita2d_color_vertex){x + width, y + height, 0.5f, bottom_right->color};
    vertices[5] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left->color};

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 6);
	return 0;
}

static int lua_vdoublegradient(lua_State *L) {
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float width = luaL_checknumber(L, 3);
	float height = luaL_checknumber(L, 4);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        12 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

	// Expecting 12 colors: top_left, top_right, center_left, center_right, bottom_left, bottom_right
	Color *top_left = (Color *)luaL_checkudata(L, 5, "color");
	Color *top_right = (Color *)luaL_checkudata(L, 6, "color");
	Color *center_left = (Color *)luaL_checkudata(L, 7, "color");
	Color *center_right = (Color *)luaL_checkudata(L, 8, "color");
	Color *bottom_left = (Color *)luaL_checkudata(L, 9, "color");
	Color *bottom_right = (Color *)luaL_checkudata(L, 10, "color");

	int half = height / 2;

	// Top half (TL -> TR -> CL) and (TR -> CR -> CL)
	vertices[0] = (vita2d_color_vertex){x, y, 0.5f, top_left->color};
	vertices[1] = (vita2d_color_vertex){x + width, y, 0.5f, top_right->color};
	vertices[2] = (vita2d_color_vertex){x, y + half, 0.5f, center_left->color};

	vertices[3] = (vita2d_color_vertex){x + width, y, 0.5f, top_right->color};
	vertices[4] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right->color};
	vertices[5] = (vita2d_color_vertex){x, y + half, 0.5f, center_left->color};

	// Bottom half (CL -> CR -> BL) and (CR -> BR -> BL)
	vertices[6] = (vita2d_color_vertex){x, y + half, 0.5f, center_left->color};
	vertices[7] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right->color};
	vertices[8] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left->color};

	vertices[9] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right->color};
	vertices[10] = (vita2d_color_vertex){x + width, y + height, 0.5f, bottom_right->color};
	vertices[11] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left->color};

	vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 12);
	return 0;
}

static int lua_hdoublegradient(lua_State *L) {
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float width = luaL_checknumber(L, 3);
	float height = luaL_checknumber(L, 4);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        12 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

	// Expecting 12 colors: left_top, center_top, right_top, left_bottom, center_bottom, right_bottom
	Color *left_top = (Color *)luaL_checkudata(L, 5, "color");
	Color *center_top = (Color *)luaL_checkudata(L, 6, "color");
	Color *right_top = (Color *)luaL_checkudata(L, 7, "color");
	Color *left_bottom = (Color *)luaL_checkudata(L, 8, "color");
	Color *center_bottom = (Color *)luaL_checkudata(L, 9, "color");
	Color *right_bottom = (Color *)luaL_checkudata(L, 10, "color");

	int half = width / 2;

	// Left half (LT -> CT -> LB) and (CT -> CB -> LB)
	vertices[0] = (vita2d_color_vertex){x, y, 0.5f, left_top->color};
	vertices[1] = (vita2d_color_vertex){x + half, y, 0.5f, center_top->color};
	vertices[2] = (vita2d_color_vertex){x, y + height, 0.5f, left_bottom->color};

	vertices[3] = (vita2d_color_vertex){x + half, y, 0.5f, center_top->color};
	vertices[4] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom->color};
	vertices[5] = (vita2d_color_vertex){x, y + height, 0.5f, left_bottom->color};

	// Right half (CT -> RT -> CB) and (RT -> RB -> CB)
	vertices[6] = (vita2d_color_vertex){x + half, y, 0.5f, center_top->color};
	vertices[7] = (vita2d_color_vertex){x + width, y, 0.5f, right_top->color};
	vertices[8] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom->color};

	vertices[9] = (vita2d_color_vertex){x + width, y, 0.5f, right_top->color};
	vertices[10] = (vita2d_color_vertex){x + width, y + height, 0.5f, right_bottom->color};
	vertices[11] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom->color};

	vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 12);
	return 0;
}

static int lua_enableclip(lua_State *L){
	bool enable = lua_toboolean(L, 1);
	if(enable) vita2d_enable_clipping();
	else vita2d_disable_clipping();
	return 0;
}

static int lua_cliprect(lua_State *L){
	int minx = luaL_checkinteger(L, 1);
	int miny = luaL_checkinteger(L, 2);
	int maxx = luaL_checkinteger(L, 3);
	int maxy = luaL_checkinteger(L, 4);
	vita2d_set_clip_rectangle(minx, miny, maxx, maxy);
	return 0;
}

static const struct luaL_Reg draw_lib[] = {
    {"text", lua_text},
	{"textwidth", lua_textwidth},
	{"textheight", lua_textheight},
    {"rect", lua_rect},
    {"circle", lua_circle},
    {"line", lua_line},
	{"pixel", lua_pixel},
	{"gradientrect", lua_gradient},
	{"hdoublegradientrect", lua_hdoublegradient},
	{"vdoublegradientrect", lua_vdoublegradient},
	{"enableclip", lua_enableclip},
	{"cliprect", lua_cliprect},
    {"swapbuffers", lua_swapbuff},
    {NULL, NULL}
};

void luaL_opendraw(lua_State *L) {
	luaL_openlib(L, "draw", draw_lib, 0);
}

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
			return luaL_error(L, "Image file type isn't accepted (must be a .png, .jpeg/.jpg, or a .bmp)");
		}
	}else{
		return luaL_error(L, "Image doesn't exist: %s", filename);
	}
    if (!image->tex) return luaL_error(L, "Failed to load image: %s", filename);
    
    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

int lua_screenimage(lua_State *L) {
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));

	image->tex = vita2d_get_current_fb();
    
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

static const struct luaL_Reg image_lib[] = {
    {"load", lua_imageload},
	{"screen", lua_screenimage},
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
    {NULL, NULL}
};

static const struct luaL_Reg image_methods[] = {
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
    {NULL, NULL}
};

void luaL_openimage(lua_State *L) {
	luaL_newmetatable(L, "image");
    lua_pushcfunction(L, lua_imagegc);
    lua_setfield(L, -2, "__gc");

	lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, image_methods, 0);
    lua_pop(L, 1);

	luaL_openlib(L, "image", image_lib, 0);
}

static int lua_loadfont(lua_State *L){
	const char *filename = luaL_checkstring(L, 1);
    Font *font = (Font *)lua_newuserdata(L, sizeof(Font));
	memset(font, 0, sizeof(Font));
	if(file_exists(filename)){
		if(string_ends_with(filename, ".pgf")){
			font->pgf = vita2d_load_custom_pgf(filename);
		}else if((string_ends_with(filename, ".pvf"))){
			font->pvf = vita2d_load_custom_pvf(filename);
		}else if(string_ends_with(filename, ".ttf") || string_ends_with(filename, ".woff")){
			font->font = vita2d_load_font_file(filename);
		}else{
			return luaL_error(L, "Font file type isn't accepted (must be a .pgf, .pvf, or a .ttf/.woff)");
		}
	}else{
		return luaL_error(L, "Font doesn't exist: %s", filename);
	}
    if (!(font->pgf || font->pvf || font->font)) return luaL_error(L, "Failed to load font: %s", filename);

	luaL_getmetatable(L, "font");
    lua_setmetatable(L, -2);
	return 1;
}

static int lua_fontgc(lua_State *L){
	Font *font = (Font *)luaL_checkudata(L, 1, "font");
    if (font->pgf != NULL) {
        vita2d_free_pgf(font->pgf);
    }else if(font->pvf != NULL){
		vita2d_free_pvf(font->pvf);
	}else if(font->font != NULL){
		vita2d_free_font(font->font);
	}
	return 0;
}

static const struct luaL_Reg font_lib[] = {
    {"load", lua_loadfont},
    {NULL, NULL}
};

void luaL_openfont(lua_State *L){
	luaL_newmetatable(L, "font");
    lua_pushcfunction(L, lua_fontgc);
    lua_setfield(L, -2, "__gc");

	luaL_openlib(L, "font", font_lib, 0);
}

static int lua_newcolor(lua_State *L) {
	int r = luaL_checkinteger(L, 1);
	int g = luaL_checkinteger(L, 2);
	int b = luaL_checkinteger(L, 3);
	int a = luaL_optinteger(L, 4, 255);
	Color *color = (Color *)lua_newuserdata(L, sizeof(Color));
	color->color = RGBA8(r, g, b, a);
	luaL_getmetatable(L, "color");
    lua_setmetatable(L, -2);
	return 1;
}

static const struct luaL_Reg color_lib[] = {
    {"new", lua_newcolor},
    {NULL, NULL}
};

void luaL_opencolor(lua_State *L) {
	luaL_newmetatable(L, "color");
	luaL_openlib(L, "color", color_lib, 0);
}

void luaL_lifelua_dofile(lua_State *L){
	bool error = false;
	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		vita2d_end_drawing();
    	vita2d_swap_buffers();
		sceAppMgrSetInfobarState(SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE, SCE_APPMGR_INFOBAR_COLOR_BLACK, SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE);
		error = true;
		if (vita_port != 0) {
			ftpvita_fini();
			vita_port = 0;
		}
		do {
			sceCtrlPeekBufferPositive(0, &pad, 1);
			sceKernelDelayThread(10000); // wait 10ms
		} while (pad.buttons != 0);

		oldpad = pad; // Reset oldpad to current state
		while(error){
			sceCtrlPeekBufferPositive(0, &pad, 1);
			vita2d_start_drawing();
    		vita2d_clear_screen();
			vita2d_pvf_draw_text(pvf, 2, 20, RGBA8(255, 255, 255, 255), 1.0f, "LifeLua has encountered an error:");
			vita2d_pvf_draw_text(pvf, 2, 40, RGBA8(255, 255, 255, 255), 1.0f, lua_tostring(L, -1) != NULL ? lua_tostring(L, -1) : "Unknown error");

			vita2d_pvf_draw_text(psexchar, 2, 80, RGBA8(255, 255, 255, 255), 1.0f, "\"");
			vita2d_pvf_draw_text(pvf, 2+vita2d_pvf_text_width(psexchar, 1.0f, "\""), 80, RGBA8(255, 255, 255, 255), 1.0f, " Retry");

			vita2d_pvf_draw_text(psexchar, 2, 100, RGBA8(255, 255, 255, 255), 1.0f, "#");
			if (vita_port == 0) {
				sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN | SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
				vita2d_pvf_draw_text(pvf, 2+vita2d_pvf_text_width(psexchar, 1.0f, "#"), 100, RGBA8(255, 255, 255, 255), 1.0f, " Enable FTP");
			}else{
				sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN | SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
				vita2d_pvf_draw_text(pvf, 2+vita2d_pvf_text_width(psexchar, 1.0f, "#"), 100, RGBA8(255, 255, 255, 255), 1.0f, " Disable FTP");
			}

			vita2d_pvf_draw_text(psexchar, 2, 120, RGBA8(255, 255, 255, 255), 1.0f, "!");
			vita2d_pvf_draw_text(pvf, 2+vita2d_pvf_text_width(psexchar, 1.0f, "!"), 120, RGBA8(255, 255, 255, 255), 1.0f, " Close app");

			if (vita_port != 0) {
				vita2d_pvf_draw_textf(pvf, 2, 160, RGBA8(255, 255, 255, 255), 1.0f, "FTP is now enabled at: %s:%u", vita_ip, vita_port);
			}

			if(!(pad.buttons == SCE_CTRL_CROSS) && (oldpad.buttons == SCE_CTRL_CROSS)){
				if (vita_port != 0) {
					ftpvita_fini();
					vita_port = 0;
				}
				error = false;
				luaL_lifelua_dofile(L); //this'll cause the app to freeze if you retry but the error doesn't change at all, NVM actually it doesn't anymore
			}
			else if(!(pad.buttons == SCE_CTRL_CIRCLE) && (oldpad.buttons == SCE_CTRL_CIRCLE)){
				if (vita_port != 0) {
					ftpvita_fini();
					vita_port = 0;
				}
				sceKernelExitProcess(0);
			}
			else if(!(pad.buttons == SCE_CTRL_SQUARE) && (oldpad.buttons == SCE_CTRL_SQUARE)){
				if(vita_port == 0){
					ftpvita_init(vita_ip, &vita_port);
					ftpvita_add_device("app0:");
					ftpvita_add_device("ux0:");
					ftpvita_add_device("ur0:");
				}else{
					ftpvita_fini();
					vita_port = 0;
				}
			};

			vita2d_end_drawing();
    		vita2d_swap_buffers();
			oldpad = pad;
		}
		/*if(!error){
			vita2d_start_drawing();
			vita2d_clear_screen();
			vita2d_end_drawing();
			vita2d_swap_buffers();

		//luaL_lifelua_dofile(L);
		}*/
	}
}

void loadPAF(){
	uint32_t ptr[0x100] = { 0 };
	ptr[0] = 0;
	ptr[1] = (uint32_t)&ptr[0];
	uint32_t scepaf_argp[] = { 0x400000, 0xEA60, 0x40000, 0, 0 };
	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(scepaf_argp), scepaf_argp, (SceSysmoduleOpt *)ptr);
}

void unloadPAF(){
	SceSysmoduleOpt opt;
	sceClibMemset(&opt.flags, 0, sizeof(opt));
	sceSysmoduleUnloadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, 0, NULL, &opt);
}

void luaL_extend(lua_State *L){
	luaL_dostring(L, "function math.range(num, min, max) return math.min(math.max(num, min), max) end\n\
					  function table.find(value, table) --Checks if an item is in an array\n\
 				      	for i, v in ipairs(table) do\n\
  							if v == value then\n\
   								return true\n\
  							end\n\
 						end\n\
 						return false\n\
						end\n\
						function table.removefirst(value, table1) --Removes the first instance of an item in an array\n\
 							for i, v in ipairs(table1) do\n\
  								if v == value then\n\
   									return table.remove(table1, i)\n\
  								end\n\
 							end\n\
						function math.round(num, idp)\n\
  							local mult = 10^(idp or 0)\n\
  							return math.floor(num * mult + 0.5) / mult\n\
						end\n\
						function math.inrange(num, min, max) return num >= min and num <= max end");
}

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_BACK);
	sceMotionStartSampling();
	sceMotionMagnetometerOn();

	loadPAF();
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SCREEN_SHOT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_JSON);
	sceSysmoduleLoadModule(SCE_SYSMODULE_MUSIC_EXPORT);
    sceSysmoduleLoadModule(SCE_SYSMODULE_PHOTO_EXPORT);
    sceSysmoduleLoadModule(SCE_SYSMODULE_VIDEO_EXPORT);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	scePromoterUtilityInit();
	SceAppUtilInitParam appUtilParam;
	SceAppUtilBootParam appUtilBootParam;
	memset(&appUtilParam, 0, sizeof(SceAppUtilInitParam));
	memset(&appUtilBootParam, 0, sizeof(SceAppUtilBootParam));
	sceAppUtilInit(&appUtilParam, &appUtilBootParam);
	sceCommonDialogConfigParamInit(&cmnDlgCfgParam);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&cmnDlgCfgParam.language);
	sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&cmnDlgCfgParam.enterButtonAssign);
	sceCommonDialogSetConfigParam(&cmnDlgCfgParam);
	sceShellUtilInitEvents(0);
	sceAppUtilPhotoMount();
	sceAppUtilMusicMount();
	sceAppUtilCacheMount();
	SceNetInitParam netInitParam;
	netInitParam.memory = malloc(1*1024*1024);
	netInitParam.size = 1*1024*1024;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);
	sceNetCtlInit();
	sceHttpInit(1*1024*1024);
	sceSslInit(1*1024*1024);

	if (sceIoDevctl("ux0:", 0x3001, NULL, 0, NULL, 0) == 0x80010030) unsafe = false;

	vita2d_init_advanced_with_msaa((1 * 1024 * 1024), SCE_GXM_MULTISAMPLE_4X);
    vita2d_set_clear_color(RGBA8(0, 0, 0, 255));
	pgf = vita2d_load_default_pgf();
	pvf = vita2d_load_default_pvf();
	psexchar = vita2d_load_custom_pvf("sa0:data/font/pvf/psexchar.pvf");

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_extend(L);
	luaL_extendos(L);
	luaL_opendraw(L);
	luaL_opencolor(L);
	luaL_opencontrols(L);
	luaL_extendio(L);
	luaL_opennetwork(L);
	luaL_opentimer(L);
	luaL_openimage(L);
	luaL_openfont(L);
	
	vita2d_start_drawing();
    vita2d_clear_screen();

	luaL_lifelua_dofile(L);

	//vita2d_end_drawing();
	//vita2d_common_dialog_update();
    //vita2d_swap_buffers();
	//sceDisplayWaitVblankStart();

	lua_close(L);
	vita2d_fini();
	vita2d_free_pgf(pgf);
	vita2d_free_pvf(pvf);
	vita2d_free_pvf(psexchar);
	sceSslTerm();
	sceHttpTerm();
	sceNetCtlTerm();
	sceNetTerm();
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SSL);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SCREEN_SHOT);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_PHOTO_EXPORT);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_VIDEO_EXPORT);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_MUSIC_EXPORT);
	scePromoterUtilityExit();
	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	sceAppUtilCacheUmount();
	sceAppUtilMusicUmount();
	sceAppUtilPhotoUmount();
	unloadPAF();

	sceMotionMagnetometerOff();
	sceMotionStopSampling();
	sceAppUtilShutdown();
	sceKernelExitProcess(0);
	return 0;
}