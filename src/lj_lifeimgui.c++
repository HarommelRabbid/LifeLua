/*
    LifeLua WIP
    ImGui library
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
#include <imgui_vita2d/imgui_vita.h>

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

static int lua_iminit(lua_State *L) {
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplVita2D_Init();
    return 0;
}

static int lua_imshutdown(lua_State *L) {
    ImGui_ImplVita2D_Shutdown();
	ImGui::DestroyContext();
    return 0;
}

static int lua_renderinit(lua_State *L) {
	ImGui_ImplVita2D_NewFrame();
	return 0;
}

static int lua_renderterm(lua_State *L) {
	ImGui::Render();
	ImGui_ImplVita2D_RenderDrawData(ImGui::GetDrawData());
	return 0;
}

static int lua_usetouch(lua_State *L) {
    bool enable = lua_toboolean(L, 1);
    bool rearenable = lua_toboolean(L, 2);
    bool indirect = lua_toboolean(L, 3);
    ImGui_ImplVita2D_TouchUsage(enable);
    ImGui_ImplVita2D_UseRearTouch(rearenable);
    ImGui_ImplVita2D_UseIndirectFrontTouch(indirect);
    return 0;
}

static int lua_usegamepad(lua_State *L) {
    bool enable = lua_toboolean(L, 1);
    ImGui_ImplVita2D_GamepadUsage(enable);
    return 0;
}

static int lua_usecursor(lua_State *L) {
    bool enable = lua_toboolean(L, 1);
    ImGui_ImplVita2D_MouseStickUsage(enable);
    return 0;
}

static int lua_setcursor(lua_State *L) {
    ImGuiMouseCursor type = luaL_checkinteger(L, 1);
    ImGui::SetMouseCursor(type);
    return 0;
}

static int lua_drawcursor(lua_State *L) {
    bool enable = lua_toboolean(L, 1);
    ImGui::GetIO().MouseDrawCursor = enable;
    return 0;
}

static int lua_darktheme(lua_State *L) {
    ImGui::StyleColorsDark();
    return 0;
}

static int lua_lighttheme(lua_State *L) {
    ImGui::StyleColorsLight();
    return 0;
}

static int lua_classictheme(lua_State *L){
    ImGui::StyleColorsClassic();
    return 0;
}

static int lua_menubarbegin(lua_State *L) {
	lua_pushboolean(L, ImGui::BeginMainMenuBar());
	return 1;
}

static int lua_menubarend(lua_State *L) {
	ImGui::EndMainMenuBar();
	return 0;
}

static int lua_menubegin(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	bool enabled = true; if(!lua_isnone(L, 2)) enabled = lua_toboolean(L, 2);
	lua_pushboolean(L, ImGui::BeginMenu(label, enabled));
	return 1;
}

static int lua_menuitem(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	bool selected = false; if(!lua_isnone(L, 2)) selected = lua_toboolean(L, 2);
	bool enabled = true; if(!lua_isnone(L, 3)) enabled = lua_toboolean(L, 3);
	lua_pushboolean(L, ImGui::MenuItem(label, nullptr, selected, enabled));
	return 1;
}

static int lua_menuend(lua_State *L) {
	ImGui::EndMenu();
	return 0;
}

static int lua_sameline(lua_State *L) {
	ImGui::SameLine();
	return 0;
}

static int lua_separator(lua_State *L) {
	ImGui::Separator();
	return 0;
}

static int lua_text(lua_State *L) {
	const char *text = luaL_checkstring(L, 1);
    ImGui::Text(text);
	return 0;
}

static int lua_disabledtext(lua_State *L) {
	const char *text = luaL_checkstring(L, 1);
	ImGui::TextDisabled(text);
	return 0;
}

static int lua_wrappedtext(lua_State *L) {
	const char *text = luaL_checkstring(L, 1);
	ImGui::TextWrapped(text);
	return 0;
}

static int lua_button(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	float sizex = luaL_optnumber(L, 2, 0);
	float sizey = luaL_optnumber(L, 3, 0);
	lua_pushboolean(L, ImGui::Button(label, ImVec2(sizex, sizey)));
	return 1;
}

static int lua_smallbutton(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	lua_pushboolean(L, ImGui::SmallButton(label));
	return 1;
}

static int lua_checkbox(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	bool status = lua_toboolean(L, 2);
	ImGui::Checkbox(label, &status);
	lua_pushboolean(L, status);
	return 1;
}

static int lua_radiobutton(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	bool status = lua_toboolean(L, 2);
	lua_pushboolean(L, ImGui::RadioButton(label, status));
	return 1;
}

static int lua_tooltip(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip(label);
	return 0;
}

static int lua_cursorpos(lua_State *L) {
    if(lua_isnone(L, 1) || lua_isnone(L, 2)){
        lua_newtable(L);
	    lua_pushnumber(L, ImGui::GetCursorPosX()); lua_setfield(L, -2, "x");
        lua_pushnumber(L, ImGui::GetCursorPosY()); lua_setfield(L, -2, "y");
    }else{
	    float x = luaL_checknumber(L, 1);
	    float y = luaL_checknumber(L, 2);
	    ImGui::SetCursorPos(ImVec2(x, y));
        return 0;
    }
	return 1;
}

static int lua_textsize(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	ImVec2 size = ImGui::CalcTextSize(label);
    lua_newtable(L);
	lua_pushnumber(L, size.x); lua_setfield(L, -2, "x");
	lua_pushnumber(L, size.y); lua_setfield(L, -2, "y");
	return 1;
}

static int lua_windowbegin(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	ImGuiWindowFlags flags = luaL_optinteger(L, 2, ImGuiWindowFlags_None);
	ImGui::Begin(label, nullptr, flags);
	return 0;
}

static int lua_nextwindowpos(lua_State *L) {
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	ImGuiCond flags = luaL_optinteger(L, 3, ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(x, y), flags);
	return 0;
}

static int lua_nextwindowsize(lua_State *L) {
	float w = luaL_checknumber(L, 1);
	float h = luaL_checknumber(L, 2);
	ImGuiCond flags = luaL_optinteger(L, 3, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(w, h), flags);
	return 0;
}

static int lua_windowfocus(lua_State *L) {
    if(lua_isnone(L, 1)) ImGui::SetWindowFocus();
    else{
        const char *name = luaL_checkstring(L, 1);
        ImGui::SetWindowFocus(name);
    }
	return 0;
}

static int lua_windowend(lua_State *L) {
	ImGui::End();
	return 0;
}

static int lua_windowrounding(lua_State *L) {
    float val = luaL_optnumber(L, 1, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, val);
    return 0;
}

static int lua_combobox(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	int idx = luaL_checkinteger(L, 2);
	lua_rawgeti(L, 3, idx);
	if (ImGui::BeginCombo(label, lua_tostring(L, -1))) {
		size_t len = lua_objlen(L, 3);
		for (int i = 1; i <= len; i++) {
			bool is_selected = i == idx;
			lua_rawgeti(L, 3, i);
			if (ImGui::Selectable(lua_tostring(L, -1), is_selected))
				idx = i;
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	lua_pushinteger(L, idx);
	return 1;
}

static int lua_listbox(lua_State *L) {
	const char *label = luaL_checkstring(L, 1);
	int idx = luaL_checkinteger(L, 2);
	uint32_t len = lua_objlen(L, 3);
	ImGui::ListBoxHeader(label, len);
	for (int i = 1; i <= len; i++) {
		bool is_selected = i == idx;
		lua_rawgeti(L, 3, i);
		if (ImGui::Selectable(lua_tostring(L, -1), is_selected))
			idx = i;
		if (is_selected)
			ImGui::SetItemDefaultFocus();
	}
	ImGui::ListBoxFooter();
	lua_pushinteger(L, idx);
	return 1;
}

static int lua_buttontextalign(lua_State *L) {
    float x = luaL_optnumber(L, 1, 0);
    float y = luaL_optnumber(L, 2, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(x, y));
    return 0;
}

static int lua_progressbar(lua_State *L) {
	float percentage = luaL_optnumber(L, 1, 0);
	float w = luaL_optnumber(L, 2, 0);
    float h = luaL_optnumber(L, 3, 0);
	ImGui::ProgressBar(percentage / 100, ImVec2(w, h));
	return 0;
}

static const struct luaL_Reg imgui_lib[] = {
	{"init", lua_iminit},
    {"shutdown", lua_imshutdown},
    {"renderinit", lua_renderinit},
    {"renderterm", lua_renderterm},
    {"touch", lua_usetouch},
    {"gamepad", lua_usegamepad},
    {"cursor", lua_usecursor},
    {"drawcursor", lua_drawcursor},
    {"setcursor", lua_setcursor},
    {"cursorposition", lua_cursorpos},
    {"darktheme", lua_darktheme},
    {"lighttheme", lua_lighttheme},
    {"classictheme", lua_classictheme},
    {"menubarbegin", lua_menubarbegin},
    {"menubarend", lua_menubarend},
    {"menubegin", lua_menubegin},
    {"menuitem", lua_menuitem},
    {"menuend", lua_menuend},
    {"sameline", lua_sameline},
    {"separator", lua_separator},
    {"text", lua_text},
    {"disabledtext", lua_disabledtext},
    {"wrappedtext", lua_wrappedtext},
    {"textsize", lua_textsize},
    {"button", lua_button},
    {"smallbutton", lua_smallbutton},
    {"buttontextalign", lua_buttontextalign},
    {"checkbox", lua_checkbox},
    {"radiobutton", lua_radiobutton},
    {"tooltip", lua_tooltip},
    {"windowrounding", lua_windowrounding},
    {"windowbegin", lua_windowbegin},
    {"windowfocus", lua_windowfocus},
    {"nextwindowposition", lua_nextwindowpos},
    {"nextwindowsize", lua_nextwindowsize},
    {"windowend", lua_windowend},
    {"combobox", lua_combobox},
    {"listbox", lua_listbox},
    {"progressbar", lua_progressbar},
    {NULL, NULL}
};

void luaL_openimgui(lua_State *L) {
	luaL_openlib(L, "imgui", imgui_lib, 0);
    luaL_pushglobalint(L, ImGuiMouseCursor_None);
    luaL_pushglobalint(L, ImGuiMouseCursor_Hand);
    luaL_pushglobalint(L, ImGuiMouseCursor_NotAllowed);
    luaL_pushglobalint(L, ImGuiMouseCursor_Arrow);
    luaL_pushglobalint(L, ImGuiMouseCursor_TextInput);
    luaL_pushglobalint(L, ImGuiMouseCursor_COUNT);
    luaL_pushglobalint(L, ImGuiMouseCursor_ResizeAll);
    luaL_pushglobalint(L, ImGuiMouseCursor_ResizeEW);
    luaL_pushglobalint(L, ImGuiMouseCursor_ResizeNESW);
    luaL_pushglobalint(L, ImGuiMouseCursor_ResizeNS);
    luaL_pushglobalint(L, ImGuiMouseCursor_ResizeNWSE);
    luaL_pushglobalint(L, ImGuiCond_Once);
    luaL_pushglobalint(L, ImGuiCond_Always);
    luaL_pushglobalint(L, ImGuiCond_None);
    luaL_pushglobalint(L, ImGuiCond_Appearing);
    luaL_pushglobalint(L, ImGuiCond_FirstUseEver);
    luaL_pushglobalint(L, ImGuiWindowFlags_None);
    luaL_pushglobalint(L, ImGuiWindowFlags_Modal);
    luaL_pushglobalint(L, ImGuiWindowFlags_MenuBar);
    luaL_pushglobalint(L, ImGuiWindowFlags_HorizontalScrollbar);
    luaL_pushglobalint(L, ImGuiWindowFlags_Popup);
    luaL_pushglobalint(L, ImGuiWindowFlags_Tooltip);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoTitleBar);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoScrollbar);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoScrollWithMouse);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoCollapse);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoMove);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoMouseInputs);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoInputs);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoFocusOnAppearing);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoBringToFrontOnFocus);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoNav);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoSavedSettings);
    luaL_pushglobalint(L, ImGuiWindowFlags_UnsavedDocument);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoResize);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoBackground);
    luaL_pushglobalint(L, ImGuiWindowFlags_NoDecoration);
    luaL_pushglobalint(L, ImGuiWindowFlags_ChildWindow);
    luaL_pushglobalint(L, ImGuiWindowFlags_ChildMenu);
    luaL_pushglobalint(L, ImGuiWindowFlags_AlwaysUseWindowPadding);
    luaL_pushglobalint(L, ImGuiWindowFlags_AlwaysVerticalScrollbar);
}