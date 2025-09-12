/*
    LifeLua WIP
    Microphone library
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

#define MIC_SAMPLERATE 16000
#define MIC_GRAIN      256

SceKernelLwMutexWork mutex;
SceKernelLwCondWork cond;
bool paused = false, micstarted = false;

static int mic_thread(SceSize args, void *argp){
    sceKernelLockLwMutex(&mutex, 1, NULL);
    while (paused) {
        sceKernelWaitLwCond(&cond, NULL);
    }
    sceKernelUnlockLwMutex(&mutex, 1);

    int ch = sceAudioInOpenPort(SCE_AUDIO_IN_PORT_TYPE_VOICE, MIC_GRAIN, MIC_SAMPLERATE, SCE_AUDIO_IN_PARAM_FORMAT_S16_MONO);

    return sceKernelExitDeleteThread(0);
}

static int lua_start(lua_State *L){
    if(micstarted) return luaL_error(L, "Microphone is already started");
    SceUID thid = sceKernelCreateThread("LifeLua Microphone Thread", &mic_thread, 0x10000100, 0x100000, 0, 0, NULL);
    sceKernelStartThread(thid, 0, NULL);
    return 0;
}

static const luaL_Reg io_lib[] = {
	{"start", lua_start},
    {NULL, NULL}
};

LUALIB_API int luaL_openmicrophone(lua_State *L) {
	luaL_register(L, "microphone", io_lib);
    return 1;
}