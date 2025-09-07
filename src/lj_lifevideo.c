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

#define FRAMEBUFFER_ALIGNMENT 0x40000

SceAvPlayerHandle avplayer;
vita2d_texture *videobuf = NULL;
SceAvPlayerFrameInfo video_frame;
int videobuf_ref = LUA_NOREF;
bool video_ready = false;

static int vaudio_thread(SceSize args, void* argp){
	int ch = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, 1024, 48000, SCE_AUDIO_OUT_MODE_STEREO);
	sceAudioOutSetConfig(ch, -1, -1, (SceAudioOutMode)-1);
	
	SceAvPlayerFrameInfo audio_frame;
	memset(&audio_frame, 0, sizeof(SceAvPlayerFrameInfo));
	
	int vol_stereo[] = {SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL};
	sceAudioOutSetVolume(ch, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH | SCE_AUDIO_VOLUME_FLAG_R_CH), vol_stereo);
	
	while (video_ready) {
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

static void *memalloc(void *p, uint32_t alignment, uint32_t size)
{
    return memalign(alignment, size);
}

static void dealloc(void *p, void *ptr)
{
    free(ptr);
}

static void *gpu_alloc(void *p, uint32_t alignment, uint32_t size)
{
    void *res = NULL;

    if (alignment < FRAMEBUFFER_ALIGNMENT)
        alignment = FRAMEBUFFER_ALIGNMENT;

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

static void gpu_dealloc(void *p, void *ptr)
{
    SceUID memblock = sceKernelFindMemBlockByAddr(ptr, 0);
    sceGxmUnmapMemory(ptr);
    sceKernelFreeMemBlock(memblock);
}

static int lua_videoload(lua_State *L){
    if (video_ready) return 0;

	const char *path = luaL_checkstring(L, 1);
	bool loop = lua_toboolean(L, 3); // 2nd param is autostart

	videobuf = (vita2d_texture *)malloc(sizeof(*videobuf));

    SceAvPlayerInitData init_data;
    memset(&init_data, 0, sizeof(SceAvPlayerInitData));
    init_data.memoryReplacement.allocate = memalloc;
    init_data.memoryReplacement.deallocate = dealloc;
    init_data.memoryReplacement.allocateTexture = gpu_alloc;
    init_data.memoryReplacement.deallocateTexture = gpu_dealloc;
    init_data.basePriority = 125;
    init_data.numOutputVideoFrameBuffers = 6;
	init_data.debugLevel = 3;
    init_data.autoStart = lua_toboolean(L, 2);
    avplayer = sceAvPlayerInit(&init_data);
	if(avplayer < 0) sceClibPrintf("Video init FAILED: 0x%X", avplayer);

	memset(&video_frame, 0, sizeof(SceAvPlayerFrameInfo));

    video_ready = true;

	int res = sceAvPlayerAddSource(avplayer, path);
	if (res < 0) luaL_error(L, "sceAvPlayerAddSource failed: 0x%08X", res);
	sceAvPlayerSetLooping(avplayer, loop);
	sceAvPlayerSetTrickSpeed(avplayer, 100);

	if(audio_active){
		vitaAudioSetChannelCallback(0, NULL, NULL);
		vitaAudioEndPre();
		vitaAudioEnd();
		audio_active = false;
	}
	SceUID audio_thread = sceKernelCreateThread("LifeLua Video Audio Thread", &vaudio_thread, 0x10000100, 0x10000, 0, 0, NULL);
	sceKernelStartThread(audio_thread, 0, NULL);
	audio_active = true;
    return 0;
}

static int lua_output(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	if(sceAvPlayerGetVideoData(avplayer, &video_frame)){
		sceGxmTextureInitLinear(
    	    &videobuf->gxm_tex,
    	    video_frame.pData,
    	    SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1,
    	    video_frame.details.video.width,
    	    video_frame.details.video.height, 0);
	}

	if(videobuf != NULL){
		float scale, startX;
    	scale = 544.0f/(float)video_frame.details.video.height;
    	startX = -((((float)video_frame.details.video.width*scale)-960.0f)/2);
		vita2d_texture_set_filters(videobuf, SCE_GXM_TEXTURE_FILTER_LINEAR, SCE_GXM_TEXTURE_FILTER_LINEAR);
    	vita2d_draw_texture_scale(videobuf, startX, 0, scale, scale);
	}
	return 0;
}

static int lua_stop(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	sceAvPlayerStop(avplayer);
	return 0;
}

static int lua_active(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	lua_pushboolean(L, sceAvPlayerIsActive(avplayer));
	return 1;
}

static int lua_vpause(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	sceAvPlayerPause(avplayer);
	return 0;
}

static int lua_vresume(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	sceAvPlayerResume(avplayer);
	return 0;
}

static int lua_time(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	lua_pushnumber(L, sceAvPlayerCurrentTime(avplayer));
	return 1;
}

static int lua_seek(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	uint64_t offset = luaL_checknumber(L, 1); // in seconds
	sceAvPlayerJumpToTime(avplayer, offset*1000);
	if(offset*1000 < sceAvPlayerCurrentTime(avplayer)) sceKernelDelayThread(100000);
	return 0;
}

static int lua_videoclose(lua_State *L){
	if(!video_ready) return luaL_error(L, "Video isn't initialized");
	sceAvPlayerClose(avplayer);
	vita2d_wait_rendering_done();
	vita2d_free_texture(videobuf);
	video_ready = false;
	return 0;
}

static const luaL_Reg video_lib[] = {
    {"load", lua_videoload},
    {"output", lua_output},
	{"active", lua_active},
	{"pause", lua_vpause},
	{"resume", lua_vresume},
	{"elapsed", lua_time},
	{"seek", lua_seek},
    {"stop", lua_stop},
	{"close", lua_videoclose},
    {NULL, NULL}
};

/*static const luaL_Reg video_methods[] = {
    {"output", lua_output},
    {"stop", lua_stop},
    {"__gc", lua_vclose},
    {NULL, NULL}
};*/

LUALIB_API int luaL_openvideo(lua_State *L) {
	luaL_register(L, "video", video_lib);
    return 1;
}