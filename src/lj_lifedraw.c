/*
    LifeLua WIP
    Draw library
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

static int lua_text(lua_State *L){
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	const char *text = luaL_checkstring(L, 3);
	Color *color = lua_tocolor(L, 4);

	float size = 1.0f;
	Font *font = NULL;

	int args = lua_gettop(L);

	if (args >= 5) {
		if (lua_isboolean(L, 5)) {
			// Case: draw.text(..., true/false)
			bool use_pvf = lua_toboolean(L, 5);
			if (args >= 6 && lua_isnumber(L, 6)) {
				size = lua_tonumber(L, 6);
			}
			if (use_pvf) vita2d_pvf_draw_text(pvf, x, y + 17.402f * size, color->color, size, text);
			else vita2d_pgf_draw_text(pgf, x, y + 17.402f * size, color->color, size, text);
		} else if (lua_isnumber(L, 5)) {
			// Case: draw.text(..., size, font | true | false)
			size = lua_tonumber(L, 5);
			if (args >= 6) {
				if (lua_isboolean(L, 6)) {
					bool use_pvf = lua_toboolean(L, 6);
					if (use_pvf) vita2d_pvf_draw_text(pvf, x, y + 17.402f * size, color->color, size, text);
					else vita2d_pgf_draw_text(pgf, x, y + 17.402f * size, color->color, size, text);
				} else if (lua_isuserdata(L, 6)) {
					font = (Font *)luaL_checkudata(L, 6, "font");
				}
			} else {
				// draw.text(..., size)
				vita2d_pgf_draw_text(pgf, x, y + 17.402f * size, color->color, size, text);
				return 0;
			}
		} else if (lua_isuserdata(L, 5)) {
			// Case: draw.text(..., font [, size])
			font = (Font *)luaL_checkudata(L, 5, "font");
			if (args >= 6 && lua_isnumber(L, 6))
				size = lua_tonumber(L, 6);
		} else {
			return luaL_typerror(L, 5, "boolean, number, or font");
		}
	} else {
		vita2d_pgf_draw_text(pgf, x, y + 17.402f * size, color->color, size, text);
		return 0;
	}

	// Final fallback: draw with custom font if set
	if (font != NULL) {
		if (font->pgf != NULL)
			vita2d_pgf_draw_text(font->pgf, x, y + 17.402f * size, color->color, size, text);
		else if (font->pvf != NULL)
			vita2d_pvf_draw_text(font->pvf, x, y + 17.402f * size, color->color, size, text);
		else if (font->font != NULL)
			vita2d_font_draw_text(font->font, x, (y - 6) + size * 24, color->color, size * 24, text);
		else
			return luaL_error(L, "Invalid font data.");
	}

	return 0;
}

static int lua_rect(lua_State *L) {
	int argc = lua_gettop(L);
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    Color *color = lua_tocolor(L, 5);
	Color *outline;
	if (argc == 6) outline = lua_tocolor(L, 6);

    vita2d_draw_rectangle(x, y, width, height, color->color);
	if (argc == 6){
		vita2d_draw_line(x-1, y, x+width, y, outline->color);
		vita2d_draw_line(x, y, x, y+height-1, outline->color);
		vita2d_draw_line(width+x, y, width+x, y+height-1, outline->color);
		vita2d_draw_line(x-1, y+height, x+width, y+height, outline->color);
	}
    return 0;
}

static int lua_circle(lua_State *L) {
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    Color *color = lua_tocolor(L, 4);

    vita2d_draw_fill_circle(x, y, radius, color->color);
    return 0;
}

static int lua_line(lua_State *L) {
    float x0 = luaL_checknumber(L, 1);
    float y0 = luaL_checknumber(L, 2);
    float x1 = luaL_checknumber(L, 3);
    float y1 = luaL_checknumber(L, 4);
    Color *color = lua_tocolor(L, 5);

    vita2d_draw_line(x0, y0, x1, y1, color->color);
    return 0;
}

static int lua_swapbuff(lua_State *L) {
	Color *color;
	if (lua_gettop(L) >= 1) color = lua_tocolor(L, 1);
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
	float size = 1.0f;
	Font *font = NULL;

	int args = lua_gettop(L);

	if (args >= 2) {
		if (lua_isboolean(L, 2)) {
			bool use_pvf = lua_toboolean(L, 2);
			if (args >= 3 && lua_isnumber(L, 3))
				size = lua_tonumber(L, 3);
			if (use_pvf) lua_pushinteger(L, vita2d_pvf_text_width(pvf, size, text));
			else lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
			return 1;
		} else if (lua_isnumber(L, 2)) {
			size = lua_tonumber(L, 2);
			if (args >= 3) {
				if (lua_isboolean(L, 3)) {
					bool use_pvf = lua_toboolean(L, 3);
					if (use_pvf) lua_pushinteger(L, vita2d_pvf_text_width(pvf, size, text));
					else lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
					return 1;
				} else if (lua_isuserdata(L, 3)) {
					font = (Font *)luaL_checkudata(L, 3, "font");
				}
			} else {
				// draw.textwidth(text, size)
				lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
				return 1;
			}
		} else if (lua_isuserdata(L, 2)) {
			font = (Font *)luaL_checkudata(L, 2, "font");
			if (args >= 3 && lua_isnumber(L, 3))
				size = lua_tonumber(L, 3);
		} else {
			return luaL_typerror(L, 2, "boolean, number, or font");
		}
	} else {
		// Only text provided
		lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
		return 1;
	}

	if (font != NULL) {
		if (font->pgf != NULL)
			lua_pushinteger(L, vita2d_pgf_text_width(font->pgf, size, text));
		else if (font->pvf != NULL)
			lua_pushinteger(L, vita2d_pvf_text_width(font->pvf, size, text));
		else if (font->font != NULL)
			lua_pushinteger(L, vita2d_font_text_width(font->font, size * 24, text));
		else
			return luaL_error(L, "Invalid font data.");
		return 1;
	}

	return luaL_error(L, "Unhandled textwidth case");
}

static int lua_textheight(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	float size = 1.0f;
	Font *font = NULL;

	int args = lua_gettop(L);

	if (args >= 2) {
		if (lua_isboolean(L, 2)) {
			bool use_pvf = lua_toboolean(L, 2);
			if (args >= 3 && lua_isnumber(L, 3))
				size = lua_tonumber(L, 3);
			if (use_pvf) lua_pushinteger(L, vita2d_pvf_text_height(pvf, size, text));
			else lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
			return 1;
		} else if (lua_isnumber(L, 2)) {
			size = lua_tonumber(L, 2);
			if (args >= 3) {
				if (lua_isboolean(L, 3)) {
					bool use_pvf = lua_toboolean(L, 3);
					if (use_pvf) lua_pushinteger(L, vita2d_pvf_text_height(pvf, size, text));
					else lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
					return 1;
				} else if (lua_isuserdata(L, 3)) {
					font = (Font *)luaL_checkudata(L, 3, "font");
				}
			} else {
				lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
				return 1;
			}
		} else if (lua_isuserdata(L, 2)) {
			font = (Font *)luaL_checkudata(L, 2, "font");
			if (args >= 3 && lua_isnumber(L, 3))
				size = lua_tonumber(L, 3);
		} else {
			return luaL_typerror(L, 2, "boolean, number, or font");
		}
	} else {
		// Only text provided
		lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
		return 1;
	}

	if (font != NULL) {
		if (font->pgf != NULL)
			lua_pushinteger(L, vita2d_pgf_text_height(font->pgf, size, text));
		else if (font->pvf != NULL)
			lua_pushinteger(L, vita2d_pvf_text_height(font->pvf, size, text));
		else if (font->font != NULL)
			lua_pushinteger(L, vita2d_font_text_height(font->font, size * 20, text));
		else
			return luaL_error(L, "Invalid font data.");
		return 1;
	}

	return luaL_error(L, "Unhandled textheight case");
}

static int lua_pixel(lua_State *L){
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	Color *color = lua_tocolor(L, 3);
	vita2d_draw_pixel(x, y, color->color);
	return 0;
}

static int lua_gradient(lua_State *L){
	float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float width = luaL_checknumber(L, 3);
    float height = luaL_checknumber(L, 4);
    Color *top_left = lua_tocolor(L, 5);
	Color *top_right = lua_tocolor(L, 6);
	Color *bottom_left = lua_tocolor(L, 7);
	Color *bottom_right = lua_tocolor(L, 8);

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
	Color *top_left = lua_tocolor(L, 5);
	Color *top_right = lua_tocolor(L, 6);
	Color *center_left = lua_tocolor(L, 7);
	Color *center_right = lua_tocolor(L, 8);
	Color *bottom_left = lua_tocolor(L, 9);
	Color *bottom_right = lua_tocolor(L, 10);

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
	Color *left_top = lua_tocolor(L, 5);
	Color *center_top = lua_tocolor(L, 6);
	Color *right_top = lua_tocolor(L, 7);
	Color *left_bottom = lua_tocolor(L, 8);
	Color *center_bottom = lua_tocolor(L, 9);
	Color *right_bottom = lua_tocolor(L, 10);

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

static int lua_triangle(lua_State *L) {
    float x1 = luaL_checknumber(L, 1);
    float y1 = luaL_checknumber(L, 2);
    float x2 = luaL_checknumber(L, 3);
    float y2 = luaL_checknumber(L, 4);
    float x3 = luaL_checknumber(L, 5);
    float y3 = luaL_checknumber(L, 6);
    Color *color = lua_tocolor(L, 7);
	Color *color1 = NULL; if(!lua_isnone(L, 8)) color1 = lua_tocolor(L, 8);
	Color *color2 = NULL; if(!lua_isnone(L, 9)) color2 = lua_tocolor(L, 9);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        3 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

	vertices[0] = (vita2d_color_vertex){x1, y1, 0.5f, color->color};
	vertices[1] = (vita2d_color_vertex){x2, y2, 0.5f, color1 ? color1->color : color->color};
	vertices[2] = (vita2d_color_vertex){x3, y3, 0.5f, color2 ? color2->color : color->color};

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 3);
    return 0;
}

static int lua_radialcircle(lua_State *L) {
    float cx = luaL_checknumber(L, 1);
    float cy = luaL_checknumber(L, 2);
    float radius = luaL_checknumber(L, 3);
    Color *center = lua_tocolor(L, 4);
    Color *edge = lua_tocolor(L, 5);
    int steps = luaL_optinteger(L, 6, 64); // Smoothness (higher = smoother)

    if (steps < 3) steps = 3;

    // Allocate vertex array (1 center + steps + 1 for wraparound)
    int total = steps + 2;
    vita2d_color_vertex *vertices = vita2d_pool_memalign(
        total * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

    // Center point
    vertices[0].x = cx;
    vertices[0].y = cy;
    vertices[0].z = 0.5f;
    vertices[0].color = center->color;

    // Edge points
    for (int i = 0; i <= steps; i++) {
        float angle = (i * 2.0f * M_PI) / steps;
        float x = cx + cosf(angle) * radius;
        float y = cy + sinf(angle) * radius;

        vertices[i + 1].x = x;
        vertices[i + 1].y = y;
        vertices[i + 1].z = 0.5f;
        vertices[i + 1].color = edge->color;
    }

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLE_FAN, vertices, total);
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

void vita2d_set_clip_circle(int x_min, int y_min, int rad)
{
	// we can only draw during a scene, but we can cache the values since they're not going to have any visible effect till the scene starts anyways
	if(true) {
		// clear the stencil buffer to 0
		sceGxmSetFrontStencilFunc(
			vita2d_get_context(),
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_ZERO,
			SCE_GXM_STENCIL_OP_ZERO,
			SCE_GXM_STENCIL_OP_ZERO,
			0xFF,
			0xFF);
		vita2d_draw_rectangle(0, 0, 960, 544, 0);
		// set the stencil to 1 in the desired region
		sceGxmSetFrontStencilFunc(
			vita2d_get_context(),
			SCE_GXM_STENCIL_FUNC_NEVER,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			SCE_GXM_STENCIL_OP_REPLACE,
			0xFF,
			0xFF);
		vita2d_draw_fill_circle(x_min, y_min, rad, 0);
		if(vita2d_get_clipping_enabled()) {
			// set the stencil function to only accept pixels where the stencil is 1
			sceGxmSetFrontStencilFunc(
				vita2d_get_context(),
				SCE_GXM_STENCIL_FUNC_EQUAL,
				SCE_GXM_STENCIL_OP_KEEP,
				SCE_GXM_STENCIL_OP_KEEP,
				SCE_GXM_STENCIL_OP_KEEP,
				0xFF,
				0xFF);
		} else {
			sceGxmSetFrontStencilFunc(
				vita2d_get_context(),
				SCE_GXM_STENCIL_FUNC_ALWAYS,
				SCE_GXM_STENCIL_OP_KEEP,
				SCE_GXM_STENCIL_OP_KEEP,
				SCE_GXM_STENCIL_OP_KEEP,
				0xFF,
				0xFF);
		}
	}
}

static int lua_clipcircle(lua_State *L){
	int minx = luaL_checkinteger(L, 1);
	int miny = luaL_checkinteger(L, 2);
	int rad = luaL_checkinteger(L, 3);
	vita2d_set_clip_circle(minx, miny, rad);
	return 0;
}

static const luaL_Reg draw_lib[] = {
    {"text", lua_text},
	{"textwidth", lua_textwidth},
	{"textheight", lua_textheight},
    {"rect", lua_rect},
    {"circle", lua_circle},
	{"triangle", lua_triangle},
	{"radialcircle", lua_radialcircle},
    {"line", lua_line},
	{"pixel", lua_pixel},
	{"gradientrect", lua_gradient},
	{"hdoublegradientrect", lua_hdoublegradient},
	{"vdoublegradientrect", lua_vdoublegradient},
	{"enableclip", lua_enableclip},
	{"cliprect", lua_cliprect},
	{"clipcircle", lua_clipcircle},
    {"swapbuffers", lua_swapbuff},
    {NULL, NULL}
};

LUALIB_API int luaL_opendraw(lua_State *L) {
	luaL_register(L, "draw", draw_lib);
    return 1;
}