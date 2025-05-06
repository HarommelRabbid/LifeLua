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
#include "include/ftpvita.h"

#include "lj_lifeinit.h"
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

#define SCE_NOTIFICATION_UTIL_TEXT_MAX                     (0x3F)

typedef struct SceNotificationUtilSendParam {
    SceWChar16 text[SCE_NOTIFICATION_UTIL_TEXT_MAX];         // must be null-terminated
    SceInt16 separator;                                      // must be 0
    SceChar8 unknown[0x3F0];
} SceNotificationUtilSendParam;

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
    vita2d_texture *tex;
} Image;

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
		vita2d_start_drawing();

		lua_getglobal(L, "LifeLuaPhotoImportDialog");
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
	{"notification", lua_notification},
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

static int lua_pixel(lua_State *L){
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	unsigned int color = luaL_checkinteger(L, 3);
	vita2d_draw_pixel(x, y, color);
	return 0;
}

static int lua_gradient(lua_State *L){
	int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int width = luaL_checkinteger(L, 3);
    int height = luaL_checkinteger(L, 4);
    unsigned int top_left = luaL_checkinteger(L, 5);
	unsigned int top_right = luaL_checkinteger(L, 6);
	unsigned int bottom_left = luaL_checkinteger(L, 7);
	unsigned int bottom_right = luaL_checkinteger(L, 8);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        6 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

    // Triangle 1: TL -> TR -> BL
    vertices[0] = (vita2d_color_vertex){x, y, 0.5f, top_left};
    vertices[1] = (vita2d_color_vertex){x + width, y, 0.5f, top_right};
    vertices[2] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left};

    // Triangle 2: TR -> BR -> BL
    vertices[3] = (vita2d_color_vertex){x + width, y, 0.5f, top_right};
    vertices[4] = (vita2d_color_vertex){x + width, y + height, 0.5f, bottom_right};
    vertices[5] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left};

    vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 6);
	return 0;
}

static int lua_vdoublegradient(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int width = luaL_checkinteger(L, 3);
	int height = luaL_checkinteger(L, 4);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        12 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

	// Expecting 12 colors: top_left, top_right, center_left, center_right, bottom_left, bottom_right
	unsigned int top_left = luaL_checkinteger(L, 5);
	unsigned int top_right = luaL_checkinteger(L, 6);
	unsigned int center_left = luaL_checkinteger(L, 7);
	unsigned int center_right = luaL_checkinteger(L, 8);
	unsigned int bottom_left = luaL_checkinteger(L, 9);
	unsigned int bottom_right = luaL_checkinteger(L, 10);

	int half = height / 2;

	// Top half (TL -> TR -> CL) and (TR -> CR -> CL)
	vertices[0] = (vita2d_color_vertex){x, y, 0.5f, top_left};
	vertices[1] = (vita2d_color_vertex){x + width, y, 0.5f, top_right};
	vertices[2] = (vita2d_color_vertex){x, y + half, 0.5f, center_left};

	vertices[3] = (vita2d_color_vertex){x + width, y, 0.5f, top_right};
	vertices[4] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right};
	vertices[5] = (vita2d_color_vertex){x, y + half, 0.5f, center_left};

	// Bottom half (CL -> CR -> BL) and (CR -> BR -> BL)
	vertices[6] = (vita2d_color_vertex){x, y + half, 0.5f, center_left};
	vertices[7] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right};
	vertices[8] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left};

	vertices[9] = (vita2d_color_vertex){x + width, y + half, 0.5f, center_right};
	vertices[10] = (vita2d_color_vertex){x + width, y + height, 0.5f, bottom_right};
	vertices[11] = (vita2d_color_vertex){x, y + height, 0.5f, bottom_left};

	vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 12);
	return 0;
}

static int lua_hdoublegradient(lua_State *L) {
	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int width = luaL_checkinteger(L, 3);
	int height = luaL_checkinteger(L, 4);

	vita2d_color_vertex *vertices = (vita2d_color_vertex *)vita2d_pool_memalign(
        12 * sizeof(vita2d_color_vertex), sizeof(vita2d_color_vertex));

	// Expecting 12 colors: left_top, center_top, right_top, left_bottom, center_bottom, right_bottom
	unsigned int left_top = luaL_checkinteger(L, 5);
	unsigned int center_top = luaL_checkinteger(L, 6);
	unsigned int right_top = luaL_checkinteger(L, 7);
	unsigned int left_bottom = luaL_checkinteger(L, 8);
	unsigned int center_bottom = luaL_checkinteger(L, 9);
	unsigned int right_bottom = luaL_checkinteger(L, 10);

	int half = width / 2;

	// Left half (LT -> CT -> LB) and (CT -> CB -> LB)
	vertices[0] = (vita2d_color_vertex){x, y, 0.5f, left_top};
	vertices[1] = (vita2d_color_vertex){x + half, y, 0.5f, center_top};
	vertices[2] = (vita2d_color_vertex){x, y + height, 0.5f, left_bottom};

	vertices[3] = (vita2d_color_vertex){x + half, y, 0.5f, center_top};
	vertices[4] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom};
	vertices[5] = (vita2d_color_vertex){x, y + height, 0.5f, left_bottom};

	// Right half (CT -> RT -> CB) and (RT -> RB -> CB)
	vertices[6] = (vita2d_color_vertex){x + half, y, 0.5f, center_top};
	vertices[7] = (vita2d_color_vertex){x + width, y, 0.5f, right_top};
	vertices[8] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom};

	vertices[9] = (vita2d_color_vertex){x + width, y, 0.5f, right_top};
	vertices[10] = (vita2d_color_vertex){x + width, y + height, 0.5f, right_bottom};
	vertices[11] = (vita2d_color_vertex){x + half, y + height, 0.5f, center_bottom};

	vita2d_draw_array(SCE_GXM_PRIMITIVE_TRIANGLES, vertices, 12);
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
    {"swapbuffers", lua_swapbuff},
    {NULL, NULL}
};

void luaL_opendraw(lua_State *L) {
	lua_newtable(L);
	luaL_setfuncs(L, draw_lib, 0);
	lua_setglobal(L, "draw");
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

static int lua_imagedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	int x = luaL_checkinteger(L, 2);
	int y = luaL_checkinteger(L, 3);
	unsigned int color;
	if(argc <= 3){
		vita2d_draw_texture(image->tex, x, y);
	}else{
		color = luaL_checkinteger(L, 4);
		vita2d_draw_texture_tint(image->tex, x, y, color);
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
    {"display", lua_imagedraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
    {NULL, NULL}
};

static const struct luaL_Reg image_methods[] = {
    {"display", lua_imagedraw},
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

	lua_newtable(L);
	luaL_setfuncs(L, image_lib, 0);
	lua_setglobal(L, "image");
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
    //{"lock", lua_lockpsbutton},
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
        return luaL_error(L, "Failed to open .SFO");

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
        return luaL_error(L, "Invalid .SFO magic");
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

		char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dirent.d_name);
		lua_pushstring(L, fullpath);
        lua_setfield(L, -2, "path");

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

static int lua_editsfo(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    const char *param = luaL_checkstring(L, 2);

    // Get the new value (could be string or number)
    int value_type = lua_type(L, 3);

    // Open file
    SceUID fd = sceIoOpen(path, SCE_O_RDWR, 0);
    if (fd < 0)
        return luaL_error(L, "Failed to open .SFO for writing");

    int file_size = sceIoLseek(fd, 0, SCE_SEEK_END);
    sceIoLseek(fd, 0, SCE_SEEK_SET);

    void *buffer = malloc(file_size);
    if (!buffer) {
        sceIoClose(fd);
        return luaL_error(L, "Failed to allocate memory");
    }

    sceIoRead(fd, buffer, file_size);

    SFOHeader *header = (SFOHeader *)buffer;
    SFOEntry *entries = (SFOEntry *)(buffer + sizeof(SFOHeader));
    char *key_table = (char *)buffer + header->key_table_offset;
    char *data_table = (char *)buffer + header->data_table_offset;

    int found = 0;

    for (int i = 0; i < header->tables_entries; i++) {
        const char *key = key_table + entries[i].key_offset;

        if (strcmp(key, param) == 0) {
            char *data = data_table + entries[i].data_offset;

            if (entries[i].param_fmt == 0x0204) { // string
                if (value_type != LUA_TSTRING) {
                    free(buffer);
                    sceIoClose(fd);
                    return luaL_error(L, "Expected a string value for field '%s'", param);
                }
                const char *newstr = lua_tostring(L, 3);
                memset(data, 0, entries[i].param_max_len); // clear
                strncpy(data, newstr, entries[i].param_max_len - 1);
                found = 1;
                break;
            } else if (entries[i].param_fmt == 0x0404) { // int32
                if (!lua_isnumber(L, 3)) {
                    free(buffer);
                    sceIoClose(fd);
                    return luaL_error(L, "Expected an integer value for field '%s'", param);
                }
                uint32_t intval = (uint32_t)lua_tonumber(L, 3);
                memcpy(data, &intval, sizeof(uint32_t));
                found = 1;
                break;
            } else {
                free(buffer);
                sceIoClose(fd);
                return luaL_error(L, "Unsupported .SFO field type for '%s'", param);
            }
        }
    }

    if (!found) {
        free(buffer);
        sceIoClose(fd);
        return luaL_error(L, "Parameter '%s' not found in .SFO", param);
    }

    sceIoLseek(fd, 0, SCE_SEEK_SET);
    sceIoWrite(fd, buffer, file_size);

    sceIoClose(fd);
    free(buffer);

    return 0;
}

static int lua_deletefolder(lua_State *L){
	const char* folder = luaL_checkstring(L, 1);
	sceIoRmdir(folder);
	return 0;
}

const char *get_filename(const char *path) {
    const char *slash = strrchr(path, '/');
    if (slash)
        return slash + 1;
    else
        return path;
}

void get_directory(const char *path, char *out_dir, size_t out_size) {
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        size_t len = last_slash - path;
        if (len >= out_size)
            len = out_size - 1;
        strncpy(out_dir, path, len);
        out_dir[len] = '\0';
    } else {
        // No slash found, return empty string
        out_dir[0] = '\0';
    }
}

static int lua_getfilename(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    const char *filename = get_filename(path);
    lua_pushstring(L, filename);
    return 1;
}

static int lua_getfolder(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    char dir[512];
    get_directory(path, dir, sizeof(dir));
    lua_pushstring(L, dir);
    return 1;
}

static const struct luaL_Reg io_lib[] = {
	{"readsfo", lua_readsfo},
	{"editsfo", lua_editsfo},
	{"exists", lua_fileexist},
	{"newfolder", lua_newfolder},
	{"deletefolder", lua_deletefolder},
	{"list", lua_list},
	{"pathstrip", lua_getfilename},
	{"filestrip", lua_getfolder},
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
	bool error = false;
	if (luaL_dofile(L, "app0:main.lua") != LUA_OK) {
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
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SCREEN_SHOT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_JSON);
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
	luaL_openimage(L);

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
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleUnloadModule(SCE_SYSMODULE_JSON);
	scePromoterUtilityExit();
	sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PROMOTER_UTIL);
	sceAppUtilPhotoUmount();
	unloadPAF();

	sceMotionMagnetometerOff();
	sceMotionStopSampling();
	sceAppUtilShutdown();
	sceKernelExitProcess(0);
	return 0;
}