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

#include <vitasdk.h>
#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>
#define str(str) #str
#define luaL_pushglobalint(L, value) do { lua_pushinteger(L, value); lua_setglobal (L, str(value)); } while(0)
#define luaL_pushglobalint_as(L, value, var) do { lua_pushinteger(L, value); lua_setglobal (L, var); } while(0)

lua_State *L;
vita2d_pgf *pgf;
vita2d_pvf *pvf;
SceCtrlData pad, oldpad;
SceCommonDialogConfigParam cmnDlgCfgParam;
bool unsafe = true;
//static uint16_t title[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
//static uint16_t initial_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
//static uint16_t input_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];

// string converting funcs
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

// lua functions
static int lua_delay(lua_State *L) {
	int secs = luaL_optinteger(L, 1, 0);
    sceKernelDelayThread(secs * 1000000); // this converts microsecs to secs
    return 0;
}
static int lua_exit(lua_State *L) {
    sceKernelExitProcess(0);
    return 0;
}

static int lua_uri(lua_State *L) {
	const char *uri_string = luaL_checkstring(L, 1);
	sceAppMgrLaunchAppByUri(0xFFFFF, (char*)uri_string);
	return 0;
}

static int lua_unsafe(lua_State *L){
	lua_pushboolean(L, unsafe);
	return 1;
}

static int lua_bootparams(lua_State *L) {
	if (!unsafe)
		return luaL_error(L, "os.launchparams() requires unsafe mode to be activated from the HENkaku settings");
	char bootparams[1024];
	bootparams[0] = 0;
	sceAppMgrGetAppParam(bootparams);
	lua_pushstring(L, bootparams);
	return 1;
}

/*
static int lua_keyboard(lua_State *L){
	char* title1 = (char*)luaL_checkstring(L, 1);
	char* default_text = (char*)luaL_optstring(L, 2, "");
	SceUInt32 type = luaL_optinteger(L, 3, SCE_IME_TYPE_DEFAULT);
	SceUInt32 mode = luaL_optinteger(L, 4, SCE_IME_TYPE_DEFAULT);
	SceUInt32 option = luaL_optinteger(L, 5, 0);
	//SceUInt32 button = luaL_optinteger(L, 7, SCE_IME_DIALOG_BUTTON_ENTER);
	SceUInt32 dialog_mode = luaL_optinteger(L, 6, SCE_IME_DIALOG_DIALOG_MODE_DEFAULT);
	SceUInt32 enter_label = luaL_optinteger(L, 7, SCE_IME_ENTER_LABEL_DEFAULT);
	SceUInt32 length = luaL_optinteger(L, 8, SCE_IME_DIALOG_MAX_TEXT_LENGTH);

	if (type > 3) return luaL_error(L, "Invalid keyboard type");
	if (mode > 1) return luaL_error(L, "Invalid keyboard mode");
	if (strlen(title1) > SCE_IME_DIALOG_MAX_TITLE_LENGTH) return luaL_error(L, "Title is too long! Try to shorten it");

	ascii2utf(initial_text, default_text);
	ascii2utf(title, title1);

	SceImeDialogParam param;
	sceImeDialogParamInit(&param);
	param.supportedLanguages = 0x0001FFFF;
	param.languagesForced = SCE_TRUE;
	param.type = type;
	param.title = title;
	param.textBoxMode = mode;
	param.maxTextLength = length;
	param.initialText = initial_text;
	param.inputTextBuffer = input_text;
	if (option > 0) param.option = option;
	param.dialogMode = dialog_mode;
	param.enterLabel = enter_label;
	sceImeDialogInit(&param);
	
	SceCommonDialogStatus status = sceImeDialogGetStatus();
	if (status == SCE_COMMON_DIALOG_STATUS_FINISHED){
		SceImeDialogResult result;
		memset(&result, 0, sizeof(SceImeDialogResult));
		sceImeDialogGetResult(&result);
		if (result.button != SCE_IME_DIALOG_BUTTON_ENTER) {
			lua_pushnil(L);
		}else{
			char res[SCE_IME_DIALOG_MAX_TEXT_LENGTH+1];
			utf2ascii(res, input_text);
			lua_pushstring(L, res);
		}
		sceImeDialogTerm();
	}
	return 1;
}
*/

/*
static int lua_message(lua_State *L) {
	const char *msg = luaL_checkstring(L, 1);
  	SceMsgDialogUserMessageParam msg_param;
  	memset(&msg_param, 0, sizeof(msg_param));
  	msg_param.buttonType = SCE_MSG_DIALOG_BUTTON_TYPE_OK;
  	msg_param.msg = (SceChar8 *)msg;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
  	param.userMsgParam = &msg_param;

	sceMsgDialogInit(&param);

	if (sceMsgDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_FINISHED){
		SceMsgDialogResult result;
		sceClibMemset(&result, 0, sizeof(SceMsgDialogResult));
		sceMsgDialogGetResult(&result);
		if (result.result == SCE_COMMON_DIALOG_RESULT_OK) lua_pushboolean(L, true);
		if (result.result == SCE_COMMON_DIALOG_RESULT_USER_CANCELED) lua_pushboolean(L, false);
		if (result.result == SCE_COMMON_DIALOG_RESULT_ABORTED) lua_pushnil(L);
	}
  	sceMsgDialogTerm();	

  	return 1;
}
*/


static const struct luaL_Reg os_lib[] = {
    {"delay", lua_delay},
	{"uri", lua_uri},
	{"unsafe", lua_unsafe},
	{"launchparams", lua_bootparams},
	//{"keyboard", lua_keyboard},
	//{"message", lua_message},
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
	//luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_DIALOG_MODE_DEFAULT);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_NONE);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_CLOSE);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_ENTER);
	//luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_DEFAULT);
	//luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_SEND);
	//luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_SEARCH);
	//luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_GO);
	//luaL_pushglobalint(L, SCE_IME_OPTION_MULTILINE);
	//luaL_pushglobalint(L, SCE_IME_OPTION_NO_AUTO_CAPITALIZATION);
	//luaL_pushglobalint(L, SCE_IME_OPTION_NO_ASSISTANCE);
}

static int lua_range(lua_State *L) {
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

static int lua_text(lua_State *L){
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	const char *text = luaL_checkstring(L, 3);
	unsigned int color = luaL_checkinteger(L, 4);
	float size = luaL_optnumber(L, 5, 1.0f);

    vita2d_pgf_draw_text(pgf, x, y+20 * size, color, size, text);
	return 0;
}

// Draw rectangle
static int lua_rect(lua_State *L) {
	int argc = lua_gettop(L);
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    unsigned int color = luaL_checkinteger(L, 5);
	unsigned int outline;
	if (argc == 6) outline = luaL_checkinteger(L, 6);

    vita2d_draw_rectangle(x, y, width, height, color);
	if (argc == 6){
		vita2d_draw_line(x-1, y, x+width, y, outline);
		vita2d_draw_line(x, y, x, y+height, outline);
		vita2d_draw_line(width+x, y, width+x, y+height, outline);
		vita2d_draw_line(x-1, y+height, x+width, y+height, outline);
	}
    return 0;
}

// Draw circle
static int lua_circle(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int radius = luaL_checkinteger(L, 3);
    unsigned int color = luaL_checkinteger(L, 4);

    vita2d_draw_fill_circle(x, y, radius, color);
    return 0;
}

// Draw line
static int lua_line(lua_State *L) {
    int x0 = luaL_checkinteger(L, 1);
    int y0 = luaL_checkinteger(L, 2);
    int x1 = luaL_checkinteger(L, 3);
    int y1 = luaL_checkinteger(L, 4);
    unsigned int color = luaL_checkinteger(L, 5);

    vita2d_draw_line(x0, y0, x1, y1, color);
    return 0;
}

static int lua_swapbuff(lua_State *L) {
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

static int lua_newcolor(lua_State *L) {
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

static int lua_lockpsbutton(lua_State *L){
	bool lock = lua_toboolean(L, 1);
	bool lockq = lua_toboolean(L, 2);
	if (lock==true){
		sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN));
	}else{
		sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN));
	}
	if(lockq==true){
		sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	}else{
		sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	}
	return 0;
}

static int lua_updatecontrols(lua_State *L){
	oldpad = pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	return 0;
}

static int lua_check(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, (pad.buttons == button));
	return 1;
}

static int lua_pressed(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, (pad.buttons == button) && !(oldpad.buttons == button));
	return 1;
}

static int lua_held(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, (pad.buttons == button) && (oldpad.buttons == button));
	return 1;
}

static int lua_released(lua_State *L){
	int button = luaL_checkinteger(L, 1);
	lua_pushboolean(L, !(pad.buttons == button) && (oldpad.buttons == button));
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

static const struct luaL_Reg controls_lib[] = {
    {"lock", lua_lockpsbutton},
	{"update", lua_updatecontrols},
	{"check", lua_check},
	{"pressed", lua_pressed},
	{"held", lua_held},
	{"released", lua_released},
	{"leftanalog", lua_analogl},
	{"rightanalog", lua_analogr},
    {NULL, NULL}
};

void luaL_opencontrols(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, controls_lib, 0);
	lua_setglobal(L, "controls");
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
}

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);

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

	SceUID fd = sceIoOpen("os0:/psp2bootconfig.skprx", SCE_O_RDONLY, 0777);
	if(fd < 0){
		unsafe = false;
	}else{
		sceIoClose(fd);
	}

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
	luaL_opencontrols(L);

	vita2d_start_drawing();
    vita2d_clear_screen();
	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		while(1){
			sceCtrlPeekBufferPositive(0, &pad, 1);
			vita2d_start_drawing();
        	vita2d_clear_screen();
			vita2d_pvf_draw_text(pvf, 2, 20, RGBA8(255, 255, 255, 255), 1.0f, "LifeLua has encountered an error:");
			vita2d_pvf_draw_text(pvf, 2, 40, RGBA8(255, 255, 255, 255), 1.0f, lua_tostring(L, -1));
			if(!(pad.buttons == SCE_CTRL_CROSS) && (oldpad.buttons == SCE_CTRL_CROSS)) break;
			vita2d_end_drawing();
    		vita2d_swap_buffers();
			oldpad = pad;
		}
	}

	lua_close(L);
	vita2d_fini();
	vita2d_free_pgf(pgf);
	vita2d_free_pvf(pvf);
	sceKernelExitProcess(0);
	return 0;
}