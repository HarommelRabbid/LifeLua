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
//#include "include/vitaaudiolib.h"

#include "lj_lifeinit.h"

#include <mpv/client.h>
#include <mpv/render_gxm.h>
#include <mpv/render.h>

typedef struct {
	vita2d_texture *img;
	mpv_render_param mpv_params[3];
	mpv_render_context *mpv_context;
	mpv_handle *mpv;
} Video;

bool video_ready = false;
static bool redraw = false;

static void mpv_render_update(void *cb_ctx) {
    redraw = true;
}

static int lua_videoload(lua_State *L){
    if (video_ready) return 0;
	Video *video = (Video *)lua_newuserdata(L, sizeof(Video));

	const char *path = luaL_checkstring(L, 1);

	video->mpv = mpv_create();
    if (!video->mpv) {
        printf("failed to create mpv context\n");
        return EXIT_FAILURE;
    }

	mpv_set_option_string(video->mpv, "terminal", "yes");
    mpv_set_option_string(video->mpv, "msg-level", "all=debug");
    mpv_set_option_string(video->mpv, "vd-lavc-threads", "4");
    mpv_set_option_string(video->mpv, "fbo-format", "rgba8");
    mpv_set_option_string(video->mpv, "hwdec", "auto");

    // Disable direct rendering, only for testing, should be enabled in actual use
    // mpv_set_option_string(mpv, "vd-lavc-dr", "no");

    // Put font file to ux0:/data/fonts/ to test libass
    mpv_set_option_string(video->mpv, "osd-fonts-dir", "ux0:/data/fonts");
    mpv_set_option_string(video->mpv, "osd-font", "Open Sans");
    mpv_set_option_string(video->mpv, "osd-msg1", "libass text");

    printf("Initialize mpv render context\n");
    mpv_gxm_init_params gxm_params = {
            .context = vita2d_get_context(),
            .shader_patcher = vita2d_get_shader_patcher(),
            .buffer_index = 0,
            .msaa = SCE_GXM_MULTISAMPLE_NONE,
    };

    mpv_render_param params[] = {
            {MPV_RENDER_PARAM_API_TYPE,        (void *) MPV_RENDER_API_TYPE_GXM},
            {MPV_RENDER_PARAM_GXM_INIT_PARAMS, &gxm_params},
            {0}
    };
    if (mpv_render_context_create(&video->mpv_context, video->mpv, params) < 0) {
        printf("failed to create mpv render context\n");
        return EXIT_FAILURE;
    }
    printf("Set update callback\n");
    mpv_render_context_set_update_callback(video->mpv_context, mpv_render_update, NULL);

    printf("Initialize mpv\n");
    if (mpv_initialize(video->mpv) < 0) {
        printf("failed to initialize mpv\n");
        return EXIT_FAILURE;
    }

	video->img = vita2d_create_empty_texture_rendertarget(960, 544, SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR);

    int flip_y = 1;
    mpv_gxm_fbo fbo = {
        .render_target = video->img->gxm_rtgt,
        .color_surface = &video->img->gxm_sfc,
        .depth_stencil_surface = &video->img->gxm_sfd,
        .w = 960,
        .h = 544,
        .format = SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_RGBA,
    };
    video->mpv_params[0] = (mpv_render_param){MPV_RENDER_PARAM_FLIP_Y, &flip_y};
	video->mpv_params[1] = (mpv_render_param){MPV_RENDER_PARAM_GXM_FBO, &fbo};
	video->mpv_params[2] = (mpv_render_param){MPV_RENDER_PARAM_INVALID, NULL};

    {
        const char *cmd[] = {"loadfile", path, "replace", NULL};
        mpv_command(video->mpv, cmd);
    }

    luaL_getmetatable(L, "video");
    lua_setmetatable(L, -2);
    return 0;
}

static const luaL_Reg video_lib[] = {
    {"load", lua_videoload},
    {NULL, NULL}
};

/*static const luaL_Reg video_methods[] = {
    {"output", lua_output},
    {"stop", lua_stop},
    {"__gc", lua_vclose},
    {NULL, NULL}
};*/

LUALIB_API int luaL_openvideo(lua_State *L) {
	luaL_newmetatable(L, "image");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, image_methods);

	luaL_register(L, "video", video_lib);
    return 1;
}