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
#include <psp2/touch.h>
#include <psp2/motion.h>
#include <psp2/kernel/processmgr.h>
#include <vita2d.h>
#include "include/ftp.h"

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>
#define str(str) #str
#define luaL_pushglobalint(L, value) do { lua_pushinteger(L, value); lua_setglobal (L, str(value)); } while(0)
#define luaL_pushglobalint_as(L, value, var) do { lua_pushinteger(L, value); lua_setglobal (L, var); } while(0)
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

lua_State *L;
vita2d_pgf *pgf;
vita2d_pvf *pvf;
vita2d_pvf *psexchar;
SceCtrlData pad, oldpad;
SceTouchData fronttouch, reartouch;
SceMotionSensorState motion;
SceCommonDialogConfigParam cmnDlgCfgParam;
static uint16_t title[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t initial_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t input_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];
bool unsafe = true;
char vita_ip[16];
unsigned short int vita_port = 0;

typedef struct {
    char magic[4];       // "\0PSF"
    uint32_t version;
    uint32_t key_table_offset;
    uint32_t data_table_offset;
    uint32_t tables_entries;
} SFOHeader;

typedef struct {
    uint16_t key_offset;
    uint16_t param_fmt;
    uint32_t param_len;
    uint32_t param_max_len;
    uint32_t data_offset;
} SFOEntry;

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

// Taken from modoru, thanks to TheFloW
void firmware_string(char string[8], unsigned int version) {
	char a = (version >> 24) & 0xf;
	char b = (version >> 20) & 0xf;
	char c = (version >> 16) & 0xf;
	char d = (version >> 12) & 0xf;

	sceClibMemset(string, 0, 8);
	string[0] = '0' + a;
	string[1] = '.';
	string[2] = '0' + b;
	string[3] = '0' + c;
	string[4] = '\0';

	if (d) {
		string[4] = '0' + d;
		string[5] = '\0';
	}
}

int file_exists(const char* path) {
	SceIoStat stat;
	int res = sceIoGetstat(path, &stat);
	if(res >= 0) return 1;
	else return 0;
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
	int flag = luaL_optinteger(L, 2, 0xFFFFF);
	sceAppMgrLaunchAppByUri(flag, (char*)uri_string);
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

static int lua_shuttersound(lua_State *L) {
	uint32_t type = (uint32_t)luaL_checkinteger(L, 1);
	if ((type > 2) || (type < 0))
		return luaL_error(L, "Invalid shutter ID");
	sceShutterSoundPlay(type);
	return 0;
}

static int lua_selfexecute(lua_State *L){
	const char* path = luaL_checkstring(L, 1);
	sceAppMgrLoadExec(path, NULL, NULL);
	return 0;
}

static int lua_keyboard(lua_State *L){
	char* title1 = (char*)luaL_checkstring(L, 1);
	char* default_text = (char*)luaL_optstring(L, 2, "");
	SceUInt32 type = luaL_optinteger(L, 3, SCE_IME_TYPE_DEFAULT);
	SceUInt32 mode = luaL_optinteger(L, 4, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT);
	SceUInt32 option = luaL_optinteger(L, 5, 0);
	//SceUInt32 button = luaL_optinteger(L, 7, SCE_IME_DIALOG_BUTTON_ENTER);
	SceUInt32 dialog_mode = luaL_optinteger(L, 6, SCE_IME_DIALOG_DIALOG_MODE_DEFAULT);
	SceUInt32 enter_label = luaL_optinteger(L, 7, SCE_IME_ENTER_LABEL_DEFAULT);
	SceUInt32 length = luaL_optinteger(L, 8, SCE_IME_DIALOG_MAX_TEXT_LENGTH);

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

	while (sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

        vita2d_end_drawing();
        vita2d_common_dialog_update();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
		vita2d_start_drawing();
    	vita2d_clear_screen(); // Clear for next frame
    }

    SceImeDialogResult result;
	sceClibMemset(&result, 0, sizeof(SceImeDialogResult));
    sceImeDialogGetResult(&result);

    if !((option == SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_CLOSE) || (option != SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_ENTER)) {
		lua_pushnil(L);
	}else{
		char res[SCE_IME_DIALOG_MAX_TEXT_LENGTH+1];
		utf2ascii(res, input_text);
		lua_pushstring(L, res);
	}

	sceImeDialogTerm();
	return 1;
}

static int lua_message(lua_State *L) {
	const char *msg = luaL_checkstring(L, 1);
	int type = luaL_optinteger(L, 2, SCE_MSG_DIALOG_BUTTON_TYPE_OK);

	SceMsgDialogUserMessageParam msg_param;
  	memset(&msg_param, 0, sizeof(msg_param));
  	msg_param.buttonType = type;
  	msg_param.msg = (SceChar8 *)msg;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
  	param.userMsgParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

        vita2d_end_drawing();
        vita2d_common_dialog_update();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
		vita2d_start_drawing();
    	vita2d_clear_screen(); // Clear for next frame
    }

	SceMsgDialogResult result;
	sceClibMemset(&result, 0, sizeof(SceMsgDialogResult));
	sceMsgDialogGetResult(&result);
	if (result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) lua_pushboolean(L, false);
	else lua_pushboolean(L, true);

	sceMsgDialogTerm();
  	return 1;
}

static int lua_realfirmware(lua_State *L) {
	char fw_str[8];
	SceKernelFwInfo info;
	info.size = sizeof(SceKernelFwInfo);
	_vshSblGetSystemSwVersion(&info);
	firmware_string(fw_str, info.version);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_spoofedfirmware(lua_State *L) {
	char fw_str[8];
	SceKernelFwInfo info;
	info.size = sizeof(SceKernelFwInfo);
	sceKernelGetSystemSwVersion(&info);
	firmware_string(fw_str, info.version);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_factoryfirmware(lua_State *L) {
	char fw_str[8];
	SceUInt32 ver;
	_vshSblAimgrGetSMI(&ver);
	firmware_string(fw_str, ver);
	lua_pushstring(L, fw_str);
	return 1;
}

static int lua_closeotherapps(lua_State *L){
	sceAppMgrDestroyOtherApp();
	return 0;
}

static int lua_closeotherapp(lua_State *L){
	const char *name = luaL_checkstring(L, 1);
	sceAppMgrDestroyAppByName(name);
	return 0;
}

static int lua_infobar(lua_State *L){
	int state = luaL_optinteger(L, 1, SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE);
	int color = luaL_optinteger(L, 2, SCE_APPMGR_INFOBAR_COLOR_BLACK);
	int transparency = luaL_optinteger(L, 3, SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE);
	sceAppMgrSetInfobarState(state, color, transparency);
	return 0;
}

static const struct luaL_Reg os_lib[] = {
    {"delay", lua_delay},
	{"uri", lua_uri},
	{"unsafe", lua_unsafe},
	{"launchparams", lua_bootparams},
	{"execute", lua_selfexecute},
	{"realfirmware", lua_realfirmware},
	{"spoofedfirmware", lua_spoofedfirmware},
	{"factoryfirmware", lua_factoryfirmware},
	{"closeotherapps", lua_closeotherapps},
	{"closeapp", lua_closeotherapp},
	{"infobar", lua_infobar},
	{"keyboard", lua_keyboard},
	{"message", lua_message},
	{"shuttersound", lua_shuttersound},
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
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE, "INFOBAR_VISIBILITY_INVISIBLE");
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE, "INFOBAR_VISIBILITY_VISIBLE");
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_COLOR_BLACK);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_COLOR_BLACK, "INFOBAR_COLOR_BLACK");
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_COLOR_WHITE);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_COLOR_WHITE, "INFOBAR_COLOR_WHITE");
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE, "INFOBAR_TRANSPARENCY_OPAQUE");
	luaL_pushglobalint(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT);
	luaL_pushglobalint_as(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT, "INFOBAR_TRANSPARENCY_TRANSLUCENT");
	luaL_pushglobalint(L, SCE_SHUTTER_SOUND_TYPE_SAVE_IMAGE);
	luaL_pushglobalint(L, SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_START);
	luaL_pushglobalint(L, SCE_SHUTTER_SOUND_TYPE_SAVE_VIDEO_END);
	luaL_pushglobalint(L, SCE_IME_TYPE_DEFAULT);
	luaL_pushglobalint(L, SCE_IME_TYPE_BASIC_LATIN);
	luaL_pushglobalint(L, SCE_IME_TYPE_NUMBER);
	luaL_pushglobalint(L, SCE_IME_TYPE_EXTENDED_NUMBER);
	luaL_pushglobalint(L, SCE_IME_TYPE_URL);
	luaL_pushglobalint(L, SCE_IME_TYPE_MAIL);
	luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT);
	luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_PASSWORD);
	luaL_pushglobalint(L, SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR);
	luaL_pushglobalint(L, SCE_IME_DIALOG_DIALOG_MODE_DEFAULT);
	luaL_pushglobalint(L, SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL);
	luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_NONE);
	luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_CLOSE);
	luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_ENTER);
	luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_DEFAULT);
	luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_SEND);
	luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_SEARCH);
	luaL_pushglobalint(L, SCE_IME_ENTER_LABEL_GO);
	luaL_pushglobalint(L, SCE_IME_OPTION_MULTILINE);
	luaL_pushglobalint(L, SCE_IME_OPTION_NO_AUTO_CAPITALIZATION);
	luaL_pushglobalint(L, SCE_IME_OPTION_NO_ASSISTANCE);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_OK);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_YESNO);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_NONE);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_OK_CANCEL);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_CANCEL);
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

    vita2d_pgf_draw_text(pgf, x, y+17.402 * size, color, size, text);
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
	vita2d_wait_rendering_done();
    vita2d_swap_buffers();
    vita2d_start_drawing();
	vita2d_set_clear_color(color);
    vita2d_clear_screen(); // Clear for next frame
    return 0;
}

static int lua_textwidth(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	float size = luaL_optnumber(L, 2, 1.0f);
	lua_pushinteger(L, vita2d_pgf_text_width(pgf, size, text));
	return 1;
}

static int lua_textheight(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	float size = luaL_optnumber(L, 2, 1.0f);
	lua_pushinteger(L, vita2d_pgf_text_height(pgf, size, text));
	return 1;
}

static const struct luaL_Reg draw_lib[] = {
    {"text", lua_text},
	{"textwidth", lua_textwidth},
	{"textheight", lua_textheight},
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
	sceMotionGetSensorState(&motion, 1);
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &fronttouch, 1);
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &reartouch, 1);
	/*
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &fronttouch, 1);
	sceTouchPeek(SCE_TOUCH_PORT_BACK, &reartouch, 1);
	lua_newtable(L); // Create fronttouch table
    for (int i = 0; i < fronttouch.reportNum; i++) {
        lua_newtable(L);

        float x = (fronttouch.report[i].x / 1920.0f) * 960.0f;
        float y = (fronttouch.report[i].y / 1088.0f) * 544.0f;

        lua_pushnumber(L, x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, y);
        lua_setfield(L, -2, "y");

        lua_pushinteger(L, fronttouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushinteger(L, fronttouch.report[i].force);
        lua_setfield(L, -2, "force");

        lua_rawseti(L, -2, i+1);
    }
    lua_setglobal(L, "fronttouch"); // fronttouch = table

    // BACK TOUCH
    lua_newtable(L); // Create backtouch table
    for (int i = 0; i < reartouch.reportNum; i++) {
        lua_newtable(L);

        float x = (reartouch.report[i].x / 1920.0f) * 960.0f;
        float y = (reartouch.report[i].y / 1088.0f) * 544.0f;

        lua_pushnumber(L, x);
        lua_setfield(L, -2, "x");

        lua_pushnumber(L, y);
        lua_setfield(L, -2, "y");

        lua_pushinteger(L, reartouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushinteger(L, reartouch.report[i].force);
        lua_setfield(L, -2, "force");

        lua_rawseti(L, -2, i+1);
    }
    lua_setglobal(L, "reartouch"); // backtouch = table
	*/
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

static int lua_enterbutton(lua_State *L){
	int enterButton;
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enterButton);
	if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CROSS) lua_pushinteger(L, SCE_CTRL_CROSS);
	else if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE) lua_pushinteger(L, SCE_CTRL_CIRCLE);
	return 1;
}

static int lua_cancelbutton(lua_State *L){
	int enterButton;
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, &enterButton);
	if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CROSS) lua_pushinteger(L, SCE_CTRL_CIRCLE);
	else if (enterButton == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE) lua_pushinteger(L, SCE_CTRL_CROSS);
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

	for (int i = 0; i < fronttouch.reportNum; i++) {
		lua_newtable(L);

		lua_pushinteger(L, lerp(fronttouch.report[i].x, 1920, 960));
		lua_setfield(L, -2, "x");

		lua_pushinteger(L, lerp(fronttouch.report[i].y, 1285, 855));
		lua_setfield(L, -2, "y");

		lua_pushinteger(L, fronttouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushinteger(L, fronttouch.report[i].force);
        lua_setfield(L, -2, "force");

		lua_rawseti(L, -2, i+1);
	}
	return 1;
}

static int lua_reartouch(lua_State *L){
	lua_newtable(L);

	for (int i = 0; i < reartouch.reportNum; i++) {
		lua_newtable(L);

		lua_pushinteger(L, lerp(reartouch.report[i].x, 1920, 960));
		lua_setfield(L, -2, "x");

		lua_pushinteger(L, lerp(reartouch.report[i].y, 1285, 855));
		lua_setfield(L, -2, "y");

		lua_pushinteger(L, reartouch.report[i].id);
        lua_setfield(L, -2, "id");

        lua_pushinteger(L, reartouch.report[i].force);
        lua_setfield(L, -2, "force");

		lua_rawseti(L, -2, i+1);
	}
	return 1;
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
	{"enterbutton", lua_enterbutton},
	{"cancelbutton", lua_cancelbutton},
	{"accelerometer", lua_accelerometer},
	{"gyroscope", lua_gyroscope},
	{"fronttouch", lua_fronttouch},
	{"reartouch", lua_reartouch},
    {NULL, NULL}
};

void luaL_opencontrols(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, controls_lib, 0);
	lua_setglobal(L, "controls");
	//lua_setfield(L, -2, "fronttouch");
	//lua_setfield(L, -2, "backtouch");
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

static int lua_fileexist(lua_State *L){
	const char* path = luaL_checkstring(L, 1);
	if (file_exists(path)) lua_pushboolean(L, true);
	else lua_pushboolean(L, false);
	return 1;
}

static int lua_newfolder(lua_State *L){
	const char* path = luaL_checkstring(L, 1);
	if(!file_exists(path)){
		char dirname[512];
		memset(dirname, 0x00, sizeof(dirname));

		for(int i = 0; i < strlen(path); i++) {
			if(path[i] == '/' || path[i] == '\\') {
				memset(dirname, 0, sizeof(dirname));
				strncpy(dirname, path, i);

				if(!file_exists(dirname)){
					sceIoMkdir(dirname, 0777);
				}	
			}
		}

		sceIoMkdir(path, 0777);
	}
	return 0;
}

static int lua_readsfo(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);

    SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
    if (fd < 0)
        return luaL_error(L, "Failed to open SFO");

    int file_size = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);

    void *buffer = malloc(file_size);
    if (!buffer) {
        sceIoClose(fd);
        return luaL_error(L, "Failed to allocate memory");
    }

    sceIoRead(fd, buffer, file_size);
    sceIoClose(fd);

    SFOHeader *header = (SFOHeader *)buffer;
    if (memcmp(header->magic, "\0PSF", 4) != 0) {
        free(buffer);
        return luaL_error(L, "Invalid SFO magic");
    }

    SFOEntry *entries = (SFOEntry *)(buffer + sizeof(SFOHeader));
    const char *key_table = (const char *)buffer + header->key_table_offset;
    const char *data_table = (const char *)buffer + header->data_table_offset;

    lua_newtable(L);

    for (int i = 0; i < header->tables_entries; i++) {
        const char *key = key_table + entries[i].key_offset;
        const void *data = data_table + entries[i].data_offset;

        if (entries[i].param_fmt == 0x0204) { // string
            lua_pushstring(L, (const char *)data);
        } else if (entries[i].param_fmt == 0x0404) { // int32
            uint32_t value;
            memcpy(&value, data, sizeof(uint32_t));
            lua_pushinteger(L, value);
        } else {
            lua_pushnil(L); // unsupported type
        }

        lua_setfield(L, -2, key);
    }

    free(buffer);
    return 1;
}

static const struct luaL_Reg io_lib[] = {
	{"readsfo", lua_readsfo},
	{"exists", lua_fileexist},
	{"newfolder", lua_newfolder},
    {NULL, NULL}
};

void luaL_extendio(lua_State *L) {
	lua_getglobal(L, "io");
	if (!lua_istable(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setglobal(L, "io");
		lua_getglobal(L, "io");
	}

	luaL_setfuncs(L, io_lib, 0);
	lua_pop(L, 1);
}

void luaL_lifelua_dofile(lua_State *L){
	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		bool error = true;
		while(error){
			sceCtrlPeekBufferPositive(0, &pad, 1);
			vita2d_start_drawing();
    		vita2d_clear_screen();
			vita2d_pvf_draw_text(pvf, 2, 20, RGBA8(255, 255, 255, 255), 1.0f, "LifeLua has encountered an error:");
			vita2d_pvf_draw_text(pvf, 2, 40, RGBA8(255, 255, 255, 255), 1.0f, lua_tostring(L, -1));

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
				//luaL_lifelua_dofile(L); this'll cause the app to freeze if you retry but the error doesn't change at all
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
		if(!error) luaL_lifelua_dofile(L);
	}
}

int main(){
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_BACK);
	sceMotionStartSampling();
	sceMotionMagnetometerOn();

	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);
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
	psexchar = vita2d_load_custom_pvf("sa0:data/font/pvf/psexchar.pvf");

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_extendos(L);
	luaL_extendmath(L);
	luaL_opendraw(L);
	luaL_opencolor(L);
	luaL_opencontrols(L);
	luaL_extendio(L);

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
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceMotionMagnetometerOff();
	sceMotionStopSampling();
	sceAppUtilShutdown();
	sceKernelExitProcess(0);
	return 0;
}