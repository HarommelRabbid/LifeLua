/*
    LifeLua WIP
    OS library extension
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
#include "include/sha1.h"

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#define SCE_NOTIFICATION_UTIL_TEXT_MAX                     (0x3F)

typedef struct SceNotificationUtilSendParam {
    SceWChar16 text[SCE_NOTIFICATION_UTIL_TEXT_MAX];         // must be null-terminated
    SceInt16 separator;                                      // must be 0
    SceChar8 unknown[0x3F0];
} SceNotificationUtilSendParam;

#define SCE_PHOTOIMPORT_DIALOG_CATEGORY_DEFAULT			(0x00000007U)
#define SCE_PHOTOIMPORT_DIALOG_CATEGORY_ALBUM_ALL		(0x00000001U)
#define SCE_PHOTOIMPORT_DIALOG_CATEGORY_ALBUM_CAMERA		(0x00000002U)
#define SCE_PHOTOIMPORT_DIALOG_CATEGORY_ALBUM_SCREENSHOT	(0x00000004U)

#define SCE_PHOTOIMPORT_DIALOG_MAX_FS_PATH			(1024)

#define SCE_PHOTOIMPORT_DIALOG_MAX_PHOTO_TITLE_LENGTH		(64)

#define SCE_PHOTOIMPORT_DIALOG_MAX_PHOTO_TITLE_SIZE	(SCE_PHOTOIMPORT_DIALOG_MAX_PHOTO_TITLE_LENGTH*4)

#define SCE_PHOTOIMPORT_DIALOG_MAX_ITEM_NUM		(1)

typedef enum ScePhotoImportDialogFormatType {
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_UNKNOWN = 0,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_JPEG,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_PNG,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_GIF,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_BMP,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_TIFF,
	SCE_PHOTOIMPORT_DIALOG_FORMAT_TYPE_MPO
} ScePhotoImportDialogFormatType;

typedef SceInt32 ScePhotoImportDialogMode;
#define SCE_PHOTOIMPORT_DIALOG_MODE_DEFAULT		(0)

typedef enum ScePhotoImportDialogOrientation {
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_UNKNOWN = 0,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_TOP_LEFT,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_TOP_RIGHT,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_BOTTOM_RIGHT,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_BOTTOM_LEFT,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_LEFT_TOP,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_RIGHT_TOP,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_RIGHT_BOTTOM,
	SCE_PHOTOIMPORT_DIALOG_ORIENTATION_LEFT_BOTTOM
} ScePhotoImportDialogOrientation;

typedef struct ScePhotoImportDialogFileDataSub {
	SceUInt32 width;
	SceUInt32 height;
	ScePhotoImportDialogFormatType format;
	ScePhotoImportDialogOrientation orientation;
	SceChar8 reserved[32];
} ScePhotoImportDialogFileDataSub;

typedef struct ScePhotoImportDialogFileData {
	SceChar8 fileName[SCE_PHOTOIMPORT_DIALOG_MAX_FS_PATH];
	SceChar8 photoTitle[SCE_PHOTOIMPORT_DIALOG_MAX_PHOTO_TITLE_SIZE];
	SceChar8 reserved[32];
} ScePhotoImportDialogFileData;

typedef struct ScePhotoImportDialogItemData {
	ScePhotoImportDialogFileData fileData;
	ScePhotoImportDialogFileDataSub dataSub;
	SceChar8 reserved[32];
} ScePhotoImportDialogItemData;

typedef struct ScePhotoImportDialogResult {
	SceInt32 result;
	SceUInt32 importedItemNum;
	SceChar8 reserved[32];
} ScePhotoImportDialogResult;

typedef struct ScePhotoImportDialogParam {
	SceUInt32 sdkVersion;
	SceCommonDialogParam commonParam;
	ScePhotoImportDialogMode mode;
	SceUInt32 visibleCategory;
	SceUInt32 itemCount;
	ScePhotoImportDialogItemData *itemData;
	SceChar8 reserved[32];
} ScePhotoImportDialogParam;

int scePhotoImportDialogInit( const ScePhotoImportDialogParam *param );
SceCommonDialogStatus scePhotoImportDialogGetStatus( void );
int scePhotoImportDialogGetResult( ScePhotoImportDialogResult* result );
int scePhotoImportDialogTerm( void );
int scePhotoImportDialogAbort( void );

static inline void scePhotoImportDialogParamInit( ScePhotoImportDialogParam *param ){

	memset( param, 0x0, sizeof(ScePhotoImportDialogParam) );
	_sceCommonDialogSetMagicNumber( &param->commonParam );
	param->sdkVersion = 0x03150021;
}

static ScePhotoImportDialogItemData s_itemData[SCE_PHOTOIMPORT_DIALOG_MAX_ITEM_NUM];

static uint16_t title[SCE_IME_DIALOG_MAX_TITLE_LENGTH];
static uint16_t initial_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH];
static uint16_t input_text[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1];

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

static int lua_delay(lua_State *L) {
	float secs = luaL_optnumber(L, 1, 0);
    sceKernelDelayThread(secs * 1000000); // this converts microsecs to secs
    return 0;
}

static int lua_delaycb(lua_State *L) {
	float secs = luaL_optnumber(L, 1, 0);
    sceKernelDelayThreadCB(secs * 1000000); // this converts microsecs to secs
    return 0;
}

static int lua_uri(lua_State *L) {
	const char *uri_string = luaL_checkstring(L, 1);
	int flag = luaL_optinteger(L, 2, 0xFFFFF);
	sceAppMgrLaunchAppByUri(flag, uri_string);
	return 0;
}

static int lua_unsafe(lua_State *L){
	lua_pushboolean(L, unsafe);
	return 1;
}

static int lua_bootparams(lua_State *L) {
	//if (!unsafe) return luaL_error(L, "os.launchparams() requires unsafe mode to be activated from the HENkaku settings");
	char bootparams[1024];
	bootparams[0] = 0;
	sceAppMgrGetAppParam(bootparams);
	lua_pushstring(L, bootparams);
	return 1;
}

static int lua_shuttersound(lua_State *L) {
	uint32_t type = (uint32_t)luaL_checkinteger(L, 1);
	/*if ((type > 2) || (type < 0))
		return luaL_error(L, "Invalid shutter ID");*/
	sceShutterSoundPlay(type);
	return 0;
}

static int lua_selfexecute(lua_State *L){
	const char* path = luaL_checkstring(L, 1);
	const char* argv = luaL_optstring(L, 2, NULL);
	sceAppMgrLoadExec(path, argv, NULL);
	return 0;
}

static int lua_keyboard(lua_State *L){
	const char* title1 = luaL_checkstring(L, 1);
	const char* default_text = luaL_optstring(L, 2, "");
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
        //vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaIMEDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

        vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
		vita2d_start_drawing();
    	vita2d_clear_screen(); // Clear for next frame
    }

    SceImeDialogResult result;
	sceClibMemset(&result, 0, sizeof(SceImeDialogResult));
    sceImeDialogGetResult(&result);

    if (!((option & SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_CLOSE) || (option != SCE_IME_OPTION_MULTILINE && result.button == SCE_IME_DIALOG_BUTTON_ENTER))) {
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
  	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	if (type == SCE_MSG_DIALOG_BUTTON_TYPE_3BUTTONS) sceClibMemset(&button_param, 0, sizeof(button_param));
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
        //vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

        vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
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
  	sceClibMemset(&msg_param, 0, sizeof(msg_param));
  	msg_param.sysMsgType = type;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_SYSTEM_MSG;
  	param.sysMsgParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        //vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaSystemMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

        vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
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
  	sceClibMemset(&msg_param, 0, sizeof(msg_param));
  	msg_param.errorCode = errorcode;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_ERROR_CODE;
  	param.errorCodeParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        //vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaErrorCodeDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

        vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
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
  	sceClibMemset(&msg_param, 0, sizeof(msg_param));
	msg_param.msg = (const SceChar8 *)msg;
	msg_param.barType = SCE_MSG_DIALOG_PROGRESSBAR_TYPE_PERCENTAGE;

  	SceMsgDialogParam param;
  	sceMsgDialogParamInit(&param);
  	_sceCommonDialogSetMagicNumber(&param.commonParam);
  	param.mode = SCE_MSG_DIALOG_MODE_PROGRESS_BAR;
  	param.progBarParam = &msg_param;
	sceMsgDialogInit(&param);

	while (sceMsgDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
        //vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaProgressMessageDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

        vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
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

static int lua_closeime(lua_State *L){
	sceImeClose();
	return 0;
}

static int lua_abortime(lua_State *L){
	sceImeDialogAbort();
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
	int runningappsint = sceAppMgrGetRunningAppIdListForShell(&runningapps, max);
	lua_newtable(L);
	for(int i = 0; i <= runningappsint; i++){
		SceUID pid = sceAppMgrGetProcessIdByAppIdForShell(runningapps[i]);
		char titleid[64];
		sceAppMgrGetNameById(pid, &titleid);
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

static int lua_suspendneeded(lua_State *L){
	lua_pushboolean(L, scePowerIsSuspendRequired());
	return 1;
}

static int lua_restart(lua_State *L){
	scePowerRequestColdReset();
	return 0;
}

static int lua_standby(lua_State *L){
	scePowerRequestStandby();
	return 0;
}

static int lua_suspend(lua_State *L){
	scePowerRequestSuspend();
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
	sceScreenShotDisable();
	if(sceScreenShotSetOverlayImage(path, offset_x, offset_y) < 0) return luaL_error(L, "Doesn't work");
	sceScreenShotEnable();
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
	if (!lua_isnil(L, 1)) param.photoTitle = (const SceWChar32 *)phototitle;
	if (!lua_isnil(L, 2)) param.gameTitle = (const SceWChar32 *)gametitle;
	if (!lua_isnil(L, 3)) param.gameComment = (const SceWChar32 *)gamecomment;
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

static int lua_cpu(lua_State *L){
	if (lua_gettop(L) >= 1){
		int freq = luaL_checkinteger(L, 1);
		scePowerSetArmClockFrequency(freq);
		return 0;
	}else{
		lua_pushinteger(L, scePowerGetArmClockFrequency());
		return 1;
	}
}

static int lua_bus(lua_State *L){
	if (lua_gettop(L) >= 1){
		int freq = luaL_checkinteger(L, 1);
		scePowerSetBusClockFrequency(freq);
		return 0;
	}else{
		lua_pushinteger(L, scePowerGetBusClockFrequency());
		return 1;
	}
}

static int lua_gpu(lua_State *L){
	if (lua_gettop(L) >= 1){
		int freq = luaL_checkinteger(L, 1);
		scePowerSetGpuXbarClockFrequency(freq);
		return 0;
	}else{
		lua_pushinteger(L, scePowerGetGpuClockFrequency());
		return 1;
	}
}

static int lua_xbar(lua_State *L){
	if (lua_gettop(L) >= 1){
		int freq = luaL_checkinteger(L, 1);
		scePowerSetGpuClockFrequency(freq);
		return 0;
	}else{
		lua_pushinteger(L, scePowerGetGpuXbarClockFrequency());
		return 1;
	}
}

static int lua_title(lua_State *L){
	char title[256];
	sceAppMgrAppParamGetString(0, 9, title , 256);
	lua_pushstring(L, title);
	return 1;
}

static int lua_titleid(lua_State *L){
	char titleid[256];
	sceAppMgrAppParamGetString(0, 12, titleid , 256);
	lua_pushstring(L, titleid);
	return 1;
}

static int lua_appexists(lua_State *L){
	const char *titleid = luaL_checkstring(L, 1);
	int res;
	lua_pushboolean(L, !scePromoterUtilityCheckExist(titleid, &res));
	return 1;
}

static void strWChar16ncpy(SceWChar16* out, const char* str2, int len)
{
    char* str1 = (char*) out;

    while (*str2 && len--)
    {
        *str1++ = *str2++;
        *str1++ = '\0';
    }
}

static int lua_notification(lua_State *L){
	const char *text = luaL_checkstring(L, 1);
	SceNotificationUtilSendParam param;
	char buf[SCE_NOTIFICATION_UTIL_TEXT_MAX];
	memset(&param, 0, sizeof(SceNotificationUtilSendParam));
	snprintf(buf, sizeof(buf), text);
	strWChar16ncpy(param.text, buf, SCE_NOTIFICATION_UTIL_TEXT_MAX);
	sceNotificationUtilSendNotification((void*) &param);
	return 0;
}

static int lua_importphoto(lua_State *L){
	int r = luaL_optinteger(L, 1, 0);
	int g = luaL_optinteger(L, 2, 0);
	int b = luaL_optinteger(L, 3, 0);
	int a = luaL_optinteger(L, 4, 0xFF);
	ScePhotoImportDialogParam pidParam;
	scePhotoImportDialogParamInit(&pidParam);

	pidParam.mode = SCE_PHOTOIMPORT_DIALOG_MODE_DEFAULT;
	pidParam.itemData = s_itemData;
	pidParam.visibleCategory = SCE_PHOTOIMPORT_DIALOG_CATEGORY_DEFAULT;
	pidParam.itemCount = 1;

	SceCommonDialogColor BgColor;

	BgColor.r = r;
	BgColor.g = g;
	BgColor.b = b;
	BgColor.a = a;

	pidParam.commonParam.bgColor = (SceCommonDialogColor*)&BgColor;

	scePhotoImportDialogInit(&pidParam);

	while (scePhotoImportDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED) {
		//vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaPhotoImportDialog");
		if (lua_isfunction(L, -1)) {
			if (lua_pcall(L, 0, 0, 0) != LUA_OK) return luaL_error(L, lua_tostring(L, -1));
		}

 		vita2d_end_drawing();
        vita2d_common_dialog_update();
		vita2d_wait_rendering_done();
        vita2d_swap_buffers();
        sceDisplayWaitVblankStart();
		vita2d_start_drawing();
    	vita2d_clear_screen(); // Clear for next frame
	}

	ScePhotoImportDialogResult pidResult;

	memset(&pidResult, 0x0, sizeof(ScePhotoImportDialogResult));
	scePhotoImportDialogGetResult(&pidResult);

	if (pidResult.result == SCE_COMMON_DIALOG_RESULT_OK) {
		for (int i = 0; i < pidResult.importedItemNum && i < SCE_PHOTOIMPORT_DIALOG_MAX_ITEM_NUM; ++i) {
			lua_pushstring(L, s_itemData[i].fileData.fileName);
		}
	}else if (pidResult.result == SCE_COMMON_DIALOG_RESULT_USER_CANCELED){
		lua_pushnil(L);
	}else if (pidResult.result == SCE_COMMON_DIALOG_RESULT_ABORTED){
		lua_pushnil(L);
	}

	scePhotoImportDialogTerm();
	return 1;
}

static int lua_photodialogabort(lua_State *L){
	scePhotoImportDialogAbort();
	return 0;
}

static int lua_vol(lua_State *L){
	if(lua_gettop(L) >= 1){
		int vol = luaL_checkinteger(L, 1);
		if (vol < 0) return luaL_error(L, "Volume number must not be lower than 0");
		else if(vol > 30) return luaL_error(L, "Volume number must not be greater than 30");
		else sceAVConfigSetSystemVol(vol);
		return 0;
	}else{
		int vol;
		sceAVConfigGetSystemVol(&vol);
		lua_pushinteger(L, vol);
		return 1;
	}
}

static int lua_mute(lua_State *L){
	sceAVConfigMuteOn();
	return 0;
}

static int lua_colorspace(lua_State *L){
	int colorspace = luaL_optinteger(L, 1, SCE_AVCONFIG_COLOR_SPACE_MODE_DEFAULT);
	sceAVConfigSetDisplayColorSpaceMode(colorspace);
	return 0;
}

static int lua_displayon(lua_State *L){
	scePowerRequestDisplayOn();
	return 0;
}

static int lua_displayoff(lua_State *L){
	scePowerRequestDisplayOff();
	return 0;
}

static int lua_exportphoto(lua_State *L){
    char outPath[256];
	const char *path = luaL_checkstring(L, 1);
	static char buf[64 * 1024];

    PhotoExportParam param;
    memset(&param, 0, sizeof(PhotoExportParam));
    int res = scePhotoExportFromFile(
        path,
        &param,
        buf,
        NULL,
        NULL,
        outPath,
        sizeof(outPath)
    );
	if (res < 0) {
		lua_pushboolean(L, false);
    } else {
		lua_pushboolean(L, true);
    }

	return 1;
}

static int lua_exportmusic(lua_State *L){
    char outPath[256];
	const char *path = luaL_checkstring(L, 1);
	static char buf[64 * 1024];

    MusicExportParam param;
    memset(&param, 0, sizeof(MusicExportParam));
    int res = sceMusicExportFromFile(
        path,
        &param,
        buf,
        NULL,
        NULL,
		NULL,
        outPath,
        sizeof(outPath)
    );
	if (res < 0) {
		lua_pushboolean(L, false);
    } else {
		lua_pushboolean(L, true);
    }

	return 1;
}

static int lua_exportvideo(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	static char buf[64 * 1024];

	VideoExportInputParam in_param;
    memset(&in_param, 0, sizeof(VideoExportInputParam));
    strncpy(in_param.path, path, 1024);
    in_param.path[1024] = '\0';

    VideoExportOutputParam out_param;
    memset(&out_param, 0, sizeof(VideoExportOutputParam));

    int res = sceVideoExportFromFile(&in_param, 1, buf, NULL, NULL, NULL, 0, &out_param);
	if (res < 0) {
		lua_pushboolean(L, false);
    } else {
		lua_pushboolean(L, true);
    }

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
	{"ime", lua_keyboard},
	{"message", lua_message},
	{"systemmessage", lua_sysmessage},
	{"errormessage", lua_errormessage},
	{"progressmessage", lua_progressmessage},
	{"progressmessagetext", lua_progmsg},
	{"setprogressmessage", lua_progset},
	{"incprogressmessage", lua_proginc},
	{"closemessage", lua_closemessage},
	{"abortmessage", lua_abortmessage},
	{"closeime", lua_closeime},
	{"abortime", lua_abortime},
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
	{"isbatterylow", lua_isbatterylow},
	{"powertick", lua_tick},
	{"powerlock", lua_powerlock},
	{"powerunlock", lua_powerunlock},
	{"cpu", lua_cpu},
	{"bus", lua_bus},
	{"gpu", lua_gpu},
	{"xbar", lua_xbar},
	{"appexists", lua_appexists},
	{"title", lua_title},
	{"titleid", lua_titleid},
	{"screenshot", lua_screenshot},
	{"screenshotoverlay", lua_screenshotoverlay},
	{"screenshotinfo", lua_screenshotinfo},
	{"unmountmountpoint", lua_mountpointunmount},
	{"getsystemevent", lua_systemevent},
	{"importphoto", lua_importphoto},
	{"abortphotoimport", lua_photodialogabort},
	{"volume", lua_vol},
	{"mute", lua_mute},
	{"colorspace", lua_colorspace},
	{"restart", lua_restart},
	{"suspend", lua_suspend},
	{"shutdown", lua_suspend},
	{"standby", lua_standby},
	{"suspendneeded", lua_suspendneeded},
	{"displayon", lua_displayon},
	{"displayoff", lua_displayoff},
	{"notification", lua_notification},
	{"photoexport", lua_exportphoto},
	{"musicexport", lua_exportmusic},
	{"videoexport", lua_exportvideo},
	{"delaycb", lua_delaycb},
    {NULL, NULL}
};

void luaL_extendos(lua_State *L) {
	luaL_openlib(L, "os", os_lib, 0);
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_VISIBILITY_INVISIBLE, "INFOBAR_VISIBILITY_INVISIBLE");
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_VISIBILITY_VISIBLE, "INFOBAR_VISIBILITY_VISIBLE");
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_COLOR_BLACK, "INFOBAR_COLOR_BLACK");
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_COLOR_WHITE, "INFOBAR_COLOR_WHITE");
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_OPAQUE, "INFOBAR_TRANSPARENCY_OPAQUE");
	luaL_pushglobalint_alsoas(L, SCE_APPMGR_INFOBAR_TRANSPARENCY_TRANSLUCENT, "INFOBAR_TRANSPARENCY_TRANSLUCENT");
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
	luaL_pushglobalint(L, SCE_AVCONFIG_COLOR_SPACE_MODE_DEFAULT);
	luaL_pushglobalint(L, SCE_AVCONFIG_COLOR_SPACE_MODE_HIGH_CONTRAST);
}