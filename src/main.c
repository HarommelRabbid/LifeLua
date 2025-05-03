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

typedef struct {
    SceUInt64 start_time;
    SceUInt64 stop_time;
    SceUInt64 pause_time;
    SceUInt64 total_paused;
    int running;
    int paused;
} Timer;

typedef struct {
	unsigned int color;
} Color;

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
static int timer_new(lua_State *L) {
    Timer *t = (Timer *)lua_newuserdata(L, sizeof(Timer));
    t->start_time = 0;
    t->stop_time = 0;
    t->running = 0;

    luaL_getmetatable(L, "timer");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_starttimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    t->start_time = sceKernelGetProcessTimeWide();
    t->stop_time = 0;
    t->pause_time = 0;
    t->total_paused = 0;
    t->running = 1;
    t->paused = 0;
    return 0;
}

static int lua_stoptimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && !t->paused) {
        t->stop_time = sceKernelGetProcessTimeWide();
    }
    t->running = 0;
    t->paused = 0;
    return 0;
}

static int lua_pausetimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && !t->paused) {
        t->pause_time = sceKernelGetProcessTimeWide();
        t->paused = 1;
    }
    return 0;
}

static int lua_resumetimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    if (t->running && t->paused) {
        SceUInt64 now = sceKernelGetProcessTimeWide();
        t->total_paused += now - t->pause_time;
        t->paused = 0;
    }
    return 0;
}

static int lua_timerelapsed(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    SceUInt64 now;
    if (t->paused)
        now = t->pause_time;
    else if (!t->running)
        now = t->stop_time;
    else
        now = sceKernelGetProcessTimeWide();

    double elapsed = (double)(now - t->start_time - t->total_paused) / 1000000.0;
    lua_pushnumber(L, elapsed);
    return 1;
}

static int lua_settimer(lua_State *L){
	Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
	SceUInt64 starting_point = luaL_optnumber(L, 2, 0);
	t->start_time = starting_point;
	return 0;
}

static int lua_resettimer(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
	SceUInt64 starting_point = luaL_optnumber(L, 2, 0);
    t->start_time = starting_point;
    t->stop_time = 0;
    t->pause_time = 0;
    t->total_paused = 0;
    t->running = 0;
    t->paused = 0;
    return 0;
}

static int lua_istimerrunning(lua_State *L) {
    Timer *t = (Timer *)luaL_checkudata(L, 1, "timer");
    lua_pushboolean(L, t->running && !(t->paused));
    return 1;
}

static const luaL_Reg timer_methods[] = {
    {"start", lua_starttimer},
    {"stop", lua_stoptimer},
    {"pause", lua_pausetimer},
    {"resume", lua_resumetimer},
    {"elapsed", lua_timerelapsed},
    {"reset", lua_resettimer},
	{"set", lua_settimer},
	{"isrunning", lua_istimerrunning},
    {NULL, NULL}
};

static const luaL_Reg timer_lib[] = {
    {"new", timer_new},
    {NULL, NULL}
};

void luaL_opentimer(lua_State *L) {
	luaL_newmetatable(L, "timer");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, timer_methods, 0);
    lua_pop(L, 1);

	lua_newtable(L);
	luaL_setfuncs(L, timer_lib, 0);
	lua_setglobal(L, "timer");
}

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
	if (!unsafe) return luaL_error(L, "os.launchparams() requires unsafe mode to be activated from the HENkaku settings");
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

		lua_getglobal(L, "LifeLuaIMEDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

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

    if (!((option == SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_CLOSE) || (option != SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_ENTER))) {
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
	const char *msg1;
	const char *msg2;
	const char *msg3;
	int size1, size2, size3;
	if (type == SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS){
		msg1 = luaL_checkstring(L, 3);
		size1 = luaL_checkinteger(L, 4);
		msg2 = luaL_checkstring(L, 5);
		size2 = luaL_checkinteger(L, 6);
		msg3 = luaL_checkstring(L, 7);
		size3 = luaL_checkinteger(L, 8);
	}

	SceMsgDialogUserMessageParam msg_param;
	SceMsgDialogButtonsParam button_param;
  	memset(&msg_param, 0, sizeof(msg_param));
	if (type == SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS) memset(&button_param, 0, sizeof(button_param));
  	msg_param.buttonType = type;
  	msg_param.msg = (SceChar8 *)msg;
	if (type == SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS){
		button_param.msg1 = msg1;
		button_param.msg2 = msg2;
		button_param.msg3 = msg3;
		button_param.fontSize1 = size1;
		button_param.fontSize2 = size2;
		button_param.fontSize3 = size3;
	}

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_USER_MSG;
	if (type == SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS) msg_param.buttonParam = &button_param;
  	param.userMsgParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

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
	if(type != SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS){
		if ((result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) || (result.buttonId == SCE_MSG_DIALOG_MODE_INVALID)) lua_pushboolean(L, false);
		else lua_pushboolean(L, true);
	}else{
		switch (result.buttonId) {
			case SCE_MSG_DIALOG_BUTTON_ID_BUTTON1:
				lua_pushstring(L, msg1); break;
			case SCE_MSG_DIALOG_BUTTON_ID_BUTTON2:
				lua_pushstring(L, msg2); break;
			case SCE_MSG_DIALOG_BUTTON_ID_BUTTON3:
				lua_pushstring(L, msg3); break;
			default:
				lua_pushnil(L);
		}
	}
	sceMsgDialogTerm();
  	return 1;
}

static int lua_sysmessage(lua_State *L) {
	int type = luaL_checkinteger(L, 1);

	SceMsgDialogSystemMessageParam msg_param;
  	memset(&msg_param, 0, sizeof(msg_param));
  	msg_param.sysMsgType = type;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_SYSTEM_MSG;
  	param.sysMsgParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaSystemMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

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
	if ((result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) || (result.buttonId == SCE_MSG_DIALOG_MODE_INVALID)) lua_pushboolean(L, false);
	else lua_pushboolean(L, true);
	sceMsgDialogTerm();
  	return 1;
}

static int lua_errormessage(lua_State *L) {
	int errorcode = luaL_checkinteger(L, 1);

	SceMsgDialogErrorCodeParam msg_param;
  	memset(&msg_param, 0, sizeof(msg_param));
  	msg_param.errorCode = errorcode;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_ERROR_CODE;
  	param.errorCodeParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaErrorCodeDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

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
	if ((result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) || (result.buttonId == SCE_MSG_DIALOG_MODE_INVALID)) lua_pushboolean(L, false);
	else lua_pushboolean(L, true);
	sceMsgDialogTerm();
  	return 1;
}

static int lua_progressmessage(lua_State *L) {
	const char *msg = luaL_checkstring(L, 1);

	SceMsgDialogProgressBarParam msg_param;
  	memset(&msg_param, 0, sizeof(msg_param));
	msg_param.msg = (const SceChar8 *)msg;
	msg_param.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
  	param.progBarParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaProgressMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

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
	if ((result.buttonId == SCE_MSG_DIALOG_BUTTON_ID_NO) || (result.buttonId == SCE_MSG_DIALOG_MODE_INVALID)) lua_pushboolean(L, false);
	else lua_pushboolean(L, true);
	sceMsgDialogTerm();
  	return 1;
}

static int lua_progmsg(lua_State *L){
	const char *msg = luaL_checkstring(L, 1);
	sceMsgDialogProgressBarSetMsg(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, (SceChar8 *)msg);
	return 0;
}

static int lua_proginc(lua_State *L){
	int inc = luaL_checkinteger(L, 1);
	sceMsgDialogProgressBarInc(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, inc);
	return 0;
}

static int lua_progset(lua_State *L){
	int inc = luaL_checkinteger(L, 1);
	sceMsgDialogProgressBarSetValue(SCE_MSG_DIALOG_PROGRESSBAR_TARGET_BAR_DEFAULT, inc);
	return 0;
}

static int lua_closemessage(lua_State *L){
	sceMsgDialogClose();
	return 0;
}

static int lua_abortmessage(lua_State *L){
	sceMsgDialogAbort();
	return 0;
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

static int lua_lock(lua_State *L){
	int argc = lua_gettop(L);
	bool enable = lua_toboolean(L, 1);
	if(enable){
		for (int i = 2; i <= argc; i++) {
			if (lua_type(L, i) == LUA_TNUMBER){
				SceShellUtilLockType type = luaL_checkinteger(L, i);
				sceShellUtilLock((SceShellUtilLockType)(type));
			}
		}
	}else{
		for (int i = 2; i <= argc; i++) {
			if (lua_type(L, i) == LUA_TNUMBER){
				SceShellUtilLockType type = luaL_checkinteger(L, i);
				sceShellUtilUnlock((SceShellUtilLockType)(type));
			}
		}
	}
	return 0;
}

static int lua_runningapps(lua_State *L){
	int max = luaL_optinteger(L, 1, 100);
	int runningapps[] = {};
	int runningappsint = sceAppMgrGetRunningAppIdListForShell(runningapps, max);
	lua_newtable(L);
	for(int i = 0; i <= runningappsint; i++){
		SceUID pid = sceAppMgrGetProcessIdByAppIdForShell(runningapps[i]);
		char titleid[64];
		sceAppMgrGetNameById(pid, titleid);
		lua_pushstring(L, titleid);
		lua_rawseti(L, -2, i+1);
	}
	return 1;
}
static int lua_ischarging(lua_State *L){
	lua_pushboolean(L, scePowerIsBatteryCharging());
	return 1;
}

static int lua_percent(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryLifePercent());
	return 1;
}

static int lua_lifetime(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryLifeTime());
	return 1;
}

static int lua_voltage(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryVolt());
	return 1;
}

static int lua_soh(lua_State *L){
	lua_pushinteger(L, scePowerGetBatterySOH());
	return 1;
}

static int lua_cyclecount(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryCycleCount());
	return 1;
}

static int lua_temperature(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryTemp() / 100); // to celsius
	return 1;
}

static int lua_capacity(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryFullCapacity());
	return 1;
}

static int lua_remainingcapacity(lua_State *L){
	lua_pushinteger(L, scePowerGetBatteryRemainCapacity());
	return 1;
}

static int lua_externalpower(lua_State *L){
	lua_pushboolean(L, scePowerIsPowerOnline());
	return 1;
}

static int lua_isbatterylow(lua_State *L){
	lua_pushboolean(L, scePowerIsLowBattery());
	return 1;
}

static int lua_tick(lua_State *L){
	int tick = luaL_optinteger(L, 1, SCE_KERNEL_POWER_TICK_DEFAULT);
	sceKernelPowerTick((SceKernelPowerTickType)tick);
	return 0;
}

static int lua_powerlock(lua_State *L){
	int tick = luaL_optinteger(L, 1, SCE_KERNEL_POWER_TICK_DEFAULT);
	sceKernelPowerLock((SceKernelPowerTickType)tick);
	return 0;
}

static int lua_powerunlock(lua_State *L){
	int tick = luaL_optinteger(L, 1, SCE_KERNEL_POWER_TICK_DEFAULT);
	sceKernelPowerUnlock((SceKernelPowerTickType)tick);
	return 0;
}

static int lua_screenshot(lua_State *L){
	bool enable = lua_toboolean(L, 1);
	if (!enable) sceScreenShotDisable();
	else sceScreenShotEnable();
	return 0;
}

static int lua_screenshotoverlay(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	int offset_x = luaL_optinteger(L, 2, 0);
	int offset_y = luaL_optinteger(L, 3, 0);
	sceScreenShotSetOverlayImage(path, offset_x, offset_y);
	return 0;
}

static int lua_screenshotinfo(lua_State *L){
	SceScreenShotParam param;
	const char *phototitle;
	const char *gametitle;
	const char *gamecomment;
	if (!lua_isnil(L, 1)) phototitle = luaL_checkstring(L, 1);
	if (!lua_isnil(L, 2)) gametitle = luaL_checkstring(L, 2);
	if (!lua_isnil(L, 3)) gamecomment = luaL_checkstring(L, 3);
	if (!lua_isnil(L, 1)) param.photoTitle = (SceWChar32 *)phototitle;
	if (!lua_isnil(L, 2)) param.gameTitle = (SceWChar32 *)gametitle;
	if (!lua_isnil(L, 3)) param.gameComment = (SceWChar32 *)gamecomment;
	sceScreenShotSetParam(&param);
	return 0;
}

static int lua_mountpointunmount(lua_State *L){
	const char *mountpoint = luaL_checkstring(L, 1);
	sceAppMgrUmount(mountpoint);
	return 0;
}

static int lua_systemevent(lua_State *L){
	SceAppMgrSystemEvent sysevent;
	sceAppMgrReceiveSystemEvent(&sysevent);
	lua_pushinteger(L, sysevent.systemEvent);
	return 1;
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
	{"systemmessage", lua_sysmessage},
	{"errormessage", lua_errormessage},
	{"progressmessage", lua_progressmessage},
	{"progressmessagetext", lua_progmsg},
	{"setprogressmessage", lua_progset},
	{"incprogressmessage", lua_proginc},
	{"closemessage", lua_closemessage},
	{"abortmessage", lua_abortmessage},
	{"shuttersound", lua_shuttersound},
	{"lock", lua_lock},
	{"runningapps", lua_runningapps},
	{"isbatterycharging", lua_ischarging},
	{"batterypercent", lua_percent},
	{"batterySOH", lua_soh},
	{"batterylifetime", lua_lifetime},
	{"batteryvoltage", lua_voltage},
	{"batterycyclecount", lua_cyclecount},
	{"batterytemperature", lua_temperature},
	{"batterycapacity", lua_capacity},
	{"remainingbatterycapacity", lua_remainingcapacity},
	{"externalbattery", lua_externalpower},
	{"batteryislow", lua_isbatterylow},
	{"powertick", lua_tick},
	{"powerlock", lua_powerlock},
	{"powerunlock", lua_powerunlock},
	{"screenshot", lua_screenshot},
	{"screenshotoverlay", lua_screenshotoverlay},
	{"screenshotinfo", lua_screenshotinfo},
	{"unmountmountpoint", lua_mountpointunmount},
	{"getsystemevent", lua_systemevent},
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
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_POWEROFF_MENU);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_UNK8);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_USB_CONNECTION);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_MC_INSERTED);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_MC_REMOVED);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_UNK80);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_UNK100);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_UNK200);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_MUSIC_PLAYER);
	luaL_pushglobalint(L, SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN_2);
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
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_NONE);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_CLOSE);
	//luaL_pushglobalint(L, SCE_IME_DIALOG_BUTTON_ENTER);
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
	luaL_pushglobalint(L, SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_FONT_SIZE_DEFAULT);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_FONT_SIZE_SMALL);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_NOSPACE);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_MAGNETIC_CALIBRATION);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_SMALL);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_WAIT_CANCEL);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_CONTINUE);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_NEED_MC_OPERATION);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_MIC_DISABLED);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_OPERATION);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_WIFI_REQUIRED_APPLICATION);
	luaL_pushglobalint(L, SCE_MSG_DIALOG_SYSMSG_TYPE_TRC_EMPTY_STORE);
	luaL_pushglobalint(L, SCE_KERNEL_POWER_TICK_DEFAULT);
	luaL_pushglobalint(L, SCE_KERNEL_POWER_TICK_DISABLE_AUTO_SUSPEND);
	luaL_pushglobalint(L, SCE_KERNEL_POWER_TICK_DISABLE_OLED_OFF);
	luaL_pushglobalint(L, SCE_KERNEL_POWER_TICK_DISABLE_OLED_DIMMING);
	luaL_pushglobalint(L, SCE_APPMGR_SYSTEMEVENT_ON_RESUME);
	luaL_pushglobalint(L, SCE_APPMGR_SYSTEMEVENT_ON_STORE_PURCHASE);
	luaL_pushglobalint(L, SCE_APPMGR_SYSTEMEVENT_ON_NP_MESSAGE_ARRIVED);
	luaL_pushglobalint(L, SCE_APPMGR_SYSTEMEVENT_ON_STORE_REDEMPTION);
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

static int lua_image(lua_State *L){
	// not too good of a implementation, but too lazy to implement custom Lua variable types right now, so I think this'll suffice for now?
	int argc = lua_gettop(L);
	const char *filename = luaL_checkstring(L, 1);
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	unsigned int color;
	vita2d_texture *image;
	if(file_exists(filename)){
		if(string_ends_with(filename, ".png")){
			image = vita2d_load_PNG_file(filename);
		}else if((string_ends_with(filename, ".jpeg")) || (string_ends_with(filename, ".jpg"))){
			image = vita2d_load_JPEG_file(filename);
		}else if(string_ends_with(filename, ".bmp")){
			image = vita2d_load_BMP_file(filename);
		}else{
			return luaL_error(L, "Image file type isn't accepted (must be a .png, .jpeg/.jpg, or a .bmp)");
		}
	}else{
		return luaL_error(L, "Image doesn't exist");
	}
	if(!(argc == 4)){
		vita2d_draw_texture(image, x, y);
	}else{
		color = luaL_checkinteger(L, 4);
		vita2d_draw_texture_tint(image, x, y, color);
	}
	//vita2d_free_texture(image);
	return 0;
}

static const struct luaL_Reg draw_lib[] = {
    {"text", lua_text},
	{"textwidth", lua_textwidth},
	{"textheight", lua_textheight},
    {"rect", lua_rect},
    {"circle", lua_circle},
    {"line", lua_line},
	{"image", lua_image},
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
	if (lock){
		sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN));
	}else{
		sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_PS_BTN));
	}
	if(lockq){
		sceShellUtilLock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	}else{
		sceShellUtilUnlock((SceShellUtilLockType)(SCE_SHELL_UTIL_LOCK_TYPE_QUICK_MENU));
	}
	return 0;
}

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

static void push_datetime(lua_State *L, const SceDateTime *dt) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             dt->year, dt->month, dt->day,
             dt->hour, dt->minute, dt->second);
    lua_pushstring(L, buffer);
}

static int lua_list(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    SceUID dir = sceIoDopen(path);
    if (dir < 0) {
        lua_pushnil(L);
        return 1;
    }

    lua_newtable(L); // files table
    int i = 1;

    SceIoDirent dirent;
    while (1) {
        memset(&dirent, 0, sizeof(SceIoDirent));
        int res = sceIoDread(dir, &dirent);
        if (res <= 0) break;

        lua_newtable(L); // single file entry

        lua_pushstring(L, dirent.d_name);
        lua_setfield(L, -2, "name");

        lua_pushboolean(L, SCE_S_ISDIR(dirent.d_stat.st_mode));
        lua_setfield(L, -2, "isafolder");

		push_datetime(L, &dirent.d_stat.st_ctime);
        lua_setfield(L, -2, "created");

        push_datetime(L, &dirent.d_stat.st_atime);
        lua_setfield(L, -2, "accessed");

        push_datetime(L, &dirent.d_stat.st_mtime);
        lua_setfield(L, -2, "modified");

        lua_pushinteger(L, dirent.d_stat.st_size);
        lua_setfield(L, -2, "size");

        lua_rawseti(L, -2, i++);
    }

    sceIoDclose(dir);
    return 1;
}

static int lua_deletefolder(lua_State *L){
	const char* folder = luaL_checkstring(L, 1);
	sceIoRmdir(folder);
	return 0;
}

static const struct luaL_Reg io_lib[] = {
	{"readsfo", lua_readsfo},
	{"exists", lua_fileexist},
	{"newfolder", lua_newfolder},
	{"deletefolder", lua_deletefolder},
	{"list", lua_list},
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

static int lua_ftp(lua_State *L){
	bool enable = lua_toboolean(L, 1);
	if (enable){
		if (vita_port == 0){
			ftpvita_init(vita_ip, &vita_port);
			lua_newtable(L);
			lua_pushstring(L, vita_ip);
			lua_setfield(L, -2, "ip");
			lua_pushinteger(L, vita_port);
			lua_setfield(L, -2, "port");
		}else{
			return luaL_error(L, "FTP was already started once");
		}
	}else{
		if (vita_port != 0) {
			ftpvita_fini();
			vita_port = 0;
			return 0;
		}else{
			return luaL_error(L, "FTP was already finished once");
		}
	}
	return 1;
}

static int lua_ftp_add(lua_State *L){
	if (vita_port != 0){
		const char* device = luaL_checkstring(L, 1);
		ftpvita_add_device(device);
	}else{
		return luaL_error(L, "FTP isn't running");
	}
	return 1;
}

static int lua_ftp_del(lua_State *L){
	if (vita_port != 0){
		const char* device = luaL_checkstring(L, 1);
		ftpvita_del_device(device);
	}else{
		return luaL_error(L, "FTP isn't running");
	}
	return 1;
}

static const struct luaL_Reg network_lib[] = {
	{"ftp", lua_ftp},
	{"ftpadddevice", lua_ftp_add},
	{"ftpremovedevice", lua_ftp_del},
    {NULL, NULL}
};

void luaL_opennetwork(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, network_lib, 0);
	lua_setglobal(L, "network");
}

void luaL_lifelua_dofile(lua_State *L){
	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
		bool error = true;
		//if (vita_port != 0) {
		//	ftpvita_fini();
		//	vita_port = 0;
		//}
		//do {
		//	sceCtrlPeekBufferPositive(0, &pad, 1);
		//	sceKernelDelayThread(10000); // wait 10ms
		//} while (pad.buttons != 0);

		//oldpad = pad; // Reset oldpad to current state
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
		if(!error){
			vita2d_start_drawing();
			vita2d_clear_screen();
			vita2d_end_drawing();
			vita2d_swap_buffers();

			luaL_lifelua_dofile(L);
		}
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
	sceSysmoduleLoadModule(SCE_SYSMODULE_SCREEN_SHOT);
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
	luaL_opennetwork(L);
	luaL_opentimer(L);

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
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_SCREEN_SHOT);
	sceMotionMagnetometerOff();
	sceMotionStopSampling();
	sceAppUtilShutdown();
	sceKernelExitProcess(0);
	return 0;
}