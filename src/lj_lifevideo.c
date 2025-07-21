/*
    LifeLua WIP
    Video library
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
#include "include/vitaaudiolib.h"

#include "lj_lifeinit.h"

#define FRAMEBUFFER_ALIGNMENT  0x40000
#define VIDEO_BUFFERING        2
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
vita2d_texture *videobuf[VIDEO_BUFFERING];
SceAvPlayerFrameInfo videoFrame[VIDEO_BUFFERING];
uint8_t videobuf_idx = 0;
SceAvPlayerHandle avplayer;
Image *lvideobuf;
static bool isPlayerReady = false;
static bool isPlaying = false;

static int audioThread(unsigned int args, void* arg){
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, 1024, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)-1);
	
	SceAvPlayerFrameInfo audio_frame;
	memset(&audio_frame, 0, sizeof(SceAvPlayerFrameInfo));
	
	// Setting audio channel volume
	int vol_stereo[] = {SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL};
	sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
	
	while (isPlayerReady) {
		if (vol_stereo[0] != SCE_AUDIO_OUT_MAX_VOL) {
			vol_stereo[0] = vol_stereo[1] = SCE_AUDIO_OUT_MAX_VOL;
			sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
		}
		if (sceAvPlayerIsActive(avplayer)) {
			if (sceAvPlayerGetAudioData(avplayer, &audio_frame)) {
				sceAudioOutSetConfig(ch, -1, audio_frame.details.audio.sampleRate, audio_frame.details.audio.channelCount == 1 ? SCE_AUDIO_OUT_MODE_MONO : SCE_AUDIO_OUT_MODE_STEREO);
				sceAudioOutOutput(ch, audio_frame.pData);
			} else {
				sceKernelDelayThread(1000);
			}
		} else {
			sceKernelDelayThread(1000);
		}
	}
	
	return sceKernelExitDeleteThread(0);
}

void *memalloc(void *p, uint32_t alignment, uint32_t size) {
	return memalign(alignment, size);
}

void dealloc(void *p, void *ptr) {
	free(ptr);
}

void *gpu_alloc(void *p, uint32_t alignment, uint32_t size) {
	void *res = NULL;
	if (alignment < FRAMEBUFFER_ALIGNMENT) {
		alignment = FRAMEBUFFER_ALIGNMENT;
	}
	size = ALIGN(size, alignment);
	SceKernelAllocMemBlockOpt opt;
	memset(&opt, 0, sizeof(opt));
	opt.size = sizeof(SceKernelAllocMemBlockOpt);
	opt.attr = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_ALIGNMENT;
	opt.alignment = alignment;
	SceUID memblock = sceKernelAllocMemBlock("Video Memblock", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, size, &opt);
	sceKernelGetMemBlockBase(memblock, &res);
	sceGxmMapMemory(res, size, (SceGxmMemoryAttribFlags)(SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE));
	return res;
}

void gpu_dealloc(void *p, void *ptr) {
	SceUID memblock = sceKernelFindMemBlockByAddr(ptr, 0);
	sceKernelFreeMemBlock(memblock);
}

void vidinit(){
	for (int i=0; i < VIDEO_BUFFERING; i++) {
		videobuf[i] = (vita2d_texture*)malloc(sizeof(vita2d_texture));
		videobuf[i]->palette_UID = 0;
		memset(&videoFrame[i], 0, sizeof(SceAvPlayerFrameInfo));
	}
	
    lvideobuf = (Image *)lua_newuserdata(L, sizeof(Image));
	
	SceAvPlayerInitData init_data;
	memset(&init_data, 0, sizeof(SceAvPlayerInitData));
	init_data.memoryReplacement.allocate            = memalloc;
	init_data.memoryReplacement.deallocate          = dealloc;
	init_data.memoryReplacement.allocateTexture 	= gpu_alloc;
	init_data.memoryReplacement.deallocateTexture 	= gpu_dealloc;
	init_data.basePriority = 0xA0;
	init_data.numOutputVideoFrameBuffers = 2;
	init_data.autoStart = true;
	avplayer = sceAvPlayerInit(&init_data);
	
	isPlayerReady = true;
}

static int lua_videoload(lua_State *L){
	const char *file = luaL_checkstring(L, 1);
	bool looping = lua_toboolean(L, 2);
	
	sceAvPlayerAddSource(avplayer, file);
	sceAvPlayerSetLooping(avplayer, looping);
	sceAvPlayerSetTrickSpeed(avplayer, 100);
	
	SceUID audio_thread = sceKernelCreateThread("LifeLua Video Sound Thread", &audioThread, 0x10000100, 0x10000, 0, 0, NULL);
	sceKernelStartThread(audio_thread, 0, NULL);
	
	isPlaying = true;
	return 0;
}

static int lua_output(lua_State *L){
	int argc = lua_gettop(L);
#ifndef SKIP_ERROR_HANDLING
	if (argc != 0) return luaL_error(L, "wrong number of arguments.");
	if (!isPlayerReady) return luaL_error(L, "you must init the player first.");
#endif
	if (sceAvPlayerGetVideoData(avplayer, &videoFrame[videobuf_idx])) {
		sceGxmTextureInitLinear(
			&videobuf[videobuf_idx]->gxm_tex,
			videoFrame[videobuf_idx].pData,
			SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
			videoFrame[videobuf_idx].details.video.width,
			videoFrame[videobuf_idx].details.video.height, 0);
		cur_text = &out_text[videobuf_idx];
		videobuf_idx = (videobuf_idx + 1) % VIDEO_BUFFERING;
	}
	lua_pushinteger(L, (uint32_t)cur_text);
	return 1;
}

static const luaL_Reg video_lib[] = {
    {"load", lua_videoload},
    {"output", lua_output},
    {"stop", lua_stop},
    {NULL, NULL}
};

static const luaL_Reg video_methods[] = {
    {"output", lua_output},
    {"stop", lua_stop},
    {"__gc", lua_videogc},
    {NULL, NULL}
};

LUALIB_API int luaL_openvideo(lua_State *L) {
	luaL_newmetatable(L, "video");

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, video_methods);

	luaL_register(L, "timer", video_lib);
    return 1;
}