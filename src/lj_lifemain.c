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
#include "include/vitaaudiolib.h"

#include "lj_lifeinit.h"

lua_State *L;
vita2d_pgf *pgf;
vita2d_pvf *pvf;
vita2d_pvf *psexchar;
SceCtrlData pad, oldpad;
SceCtrlActuator actuators[4];
SceTouchData fronttouch, reartouch;
SceMotionSensorState motion;
SceCommonDialogConfigParam cmnDlgCfgParam;
bool unsafe = true;
char vita_ip[16];
unsigned short int vita_port = 0;

int string_ends_with(const char * str, const char * suffix){
	int str_len = strlen(str);
	int suffix_len = strlen(suffix);

	return 
	(str_len >= suffix_len) &&
	(0 == strcmp(str + (str_len-suffix_len), suffix));
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
		sceCtrlPeekBufferPositive(0, &pad, 1);
		//sceKernelDelayThread(10000); // wait 10ms

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
				lua_close(L);
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

void luaL_extend(lua_State *L){
	if(
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
						end\n\
						function math.round(num, idp)\n\
  							local mult = 10^(idp or 0)\n\
  							return math.floor(num * mult + 0.5) / mult\n\
						end\n\
						function math.inrange(num, min, max) return num >= min and num <= max end\n\
						function os.installvpk(path, head)\n\
							io.extract(path, \"ux0:data/ll_temp\")\n\
							if not head then os.installdir(\"ux0:data/ll_temp\")\n\
							else os.installdir(\"ux0:data/ll_temp\", true) end\n\
						end") != LUA_OK) sceClibPrintf("LL error: %s", lua_tostring(L, -1));
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
	sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SCREEN_SHOT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_NOTIFICATION_UTIL);
	sceSysmoduleLoadModule(SCE_SYSMODULE_JSON);
	sceSysmoduleLoadModule(SCE_SYSMODULE_MUSIC_EXPORT);
    sceSysmoduleLoadModule(SCE_SYSMODULE_PHOTO_EXPORT);
    sceSysmoduleLoadModule(SCE_SYSMODULE_VIDEO_EXPORT);
	sceSysmoduleLoadModule(SCE_SYSMODULE_SQLITE);
    sqlite3_rw_init();
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
	luaL_openutf8(L);
	luaL_extendos(L);
	luaL_opendraw(L);
	luaL_opencolor(L);
	luaL_opencontrols(L);
	luaL_extendio(L);
	luaL_opennetwork(L);
	luaL_opentimer(L);
	luaL_openimage(L);
	luaL_openfont(L);
	luaL_opensqlite3(L);
	luaL_opencamera(L);
	luaL_openthread(L);
	//luaL_openimgui(L);
	luaL_openjson(L);
	luaL_openaudio(L);
	luaL_extend(L);
	
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
	sqlite3_rw_exit();
	if(audio_active){
		vitaAudioSetChannelCallback(0, NULL, NULL);
		vitaAudioEndPre();
		vitaAudioEnd();
	}
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
    sceSysmoduleUnloadModule(SCE_SYSMODULE_SQLITE);
	sceAppUtilCacheUmount();
	sceAppUtilMusicUmount();
	sceAppUtilPhotoUmount();

	sceMotionMagnetometerOff();
	sceMotionStopSampling();
	sceAppUtilShutdown();
	sceKernelExitProcess(0);
	return 0;
}