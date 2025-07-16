/*
    LifeLua WIP
    Image library
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
#include <quirc.h>

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/qrcodegen.h"
#include <png.h>
#include <libimagequant.h>

#include "lj_lifeinit.h"

static int lua_imageload(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
    
    if(file_exists(filename)){
        SceUID file = sceIoOpen(filename, SCE_O_RDONLY, 0777);
	    uint16_t magic;
	    sceIoRead(file, &magic, 2);
	    sceIoClose(file);
		if(magic == 0x5089){
			image->tex = vita2d_load_PNG_file(filename);
		}else if(magic == 0xD8FF){
			image->tex = vita2d_load_JPEG_file(filename);
		}else if(magic == 0x4D42){
			image->tex = vita2d_load_BMP_file(filename);
		}else{
			lua_pushnil(L);
			//return luaL_error(L, "Image file type isn't accepted (must be a .png, .jpeg/.jpg, or a .bmp)");
		}
	}else{
		lua_pushnil(L);
	}
    if (!image->tex) /*return luaL_error(L, "Failed to load image: %s", filename)*/lua_pushnil(L);
    
    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_newimage(lua_State *L) {
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
	unsigned int w = luaL_checkinteger(L, 1);
	unsigned int h = luaL_checkinteger(L, 2);

	vita2d_texture_set_alloc_memblock_type(SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW);

	image->tex = vita2d_create_empty_texture_rendertarget(w, h, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);
	if(lua_gettop(L) >= 3){
		Color *color = lua_tocolor(L, 3);
		sceClibMemset(vita2d_texture_get_datap(image->tex), color->color, vita2d_texture_get_stride(image->tex) * h);
	}else sceClibMemset(vita2d_texture_get_datap(image->tex), 0xFFFFFFFF, vita2d_texture_get_stride(image->tex) * h);

    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_screenimage(lua_State *L) {
    Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
	// Get framebuffer (ABGR)
    uint32_t *fb = (uint32_t *)vita2d_get_current_fb();

    // Create a texture (RGBA8)
	image->tex = vita2d_create_empty_texture_rendertarget(960, 544, SCE_GXM_TEXTURE_FORMAT_A8B8G8R8);

    // Copy framebuffer into texture
    sceClibMemcpy(vita2d_texture_get_datap(image->tex), fb, 960 * 544 * sizeof(uint32_t));

    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_qr(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    enum qrcodegen_Ecc errcor;
    int min_version, max_version;
    int mask;
    bool boostEcl = true;
	Color *bg_color = NULL;
	Color *fg_color = NULL;
    int scale;
    int border;

    // text (required)
    lua_getfield(L, 1, "text");
    const char *text = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // error_correction
    lua_getfield(L, 1, "error_correction");
    errcor = luaL_optinteger(L, -1, qrcodegen_Ecc_MEDIUM);
    lua_pop(L, 1);

    // min_version
    lua_getfield(L, 1, "min_version");
    min_version = luaL_optinteger(L, -1, qrcodegen_VERSION_MIN);
    lua_pop(L, 1);

    // max_version
    lua_getfield(L, 1, "max_version");
    max_version = luaL_optinteger(L, -1, qrcodegen_VERSION_MAX);
    lua_pop(L, 1);

    // mask
    lua_getfield(L, 1, "mask");
    mask = luaL_optinteger(L, -1, qrcodegen_Mask_AUTO);
    lua_pop(L, 1);

	// boostEcl
    lua_getfield(L, 1, "boostEcl");
    if (lua_isboolean(L, -1)) boostEcl = lua_toboolean(L, -1);
    lua_pop(L, 1);

	// bg_color
    lua_getfield(L, 1, "bg_color");
    if ((Color *)luaL_testudata(L, -1, "color")) bg_color = lua_tocolor(L, -1);
    lua_pop(L, 1);

	// fg_color
    lua_getfield(L, 1, "fg_color");
    if ((Color *)luaL_testudata(L, -1, "color")) fg_color = lua_tocolor(L, -1);
    lua_pop(L, 1);

	// scale
    lua_getfield(L, 1, "scale");
    scale = luaL_optinteger(L, -1, 4);
    lua_pop(L, 1);

	// border in modules
    lua_getfield(L, 1, "border");
    border = luaL_optinteger(L, -1, 2);
    lua_pop(L, 1);

    // Generate QR
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];

    if (!qrcodegen_encodeText(text, tempBuffer, qrcode, errcor,
                              min_version, max_version, mask, boostEcl)) return luaL_error(L, "Failed to generate QR code");

    int size = qrcodegen_getSize(qrcode);
    int tex_size = (size + border * 2) * scale;

    vita2d_texture *tex = vita2d_create_empty_texture(tex_size, tex_size);

    //vita2d_texture_set_filters(tex, SCE_GXM_TEXTURE_FILTER_POINT, SCE_GXM_TEXTURE_FILTER_POINT);
    uint32_t *data = vita2d_texture_get_datap(tex);
	unsigned int pitch = vita2d_texture_get_stride(tex) / 4;

    for (int y = 0; y < tex_size; ++y) {
        for (int x = 0; x < tex_size; ++x) {
            int module_x = (x / scale) - border;
            int module_y = (y / scale) - border;
            bool black = (module_x >= 0 && module_x < size &&
                          module_y >= 0 && module_y < size &&
                          qrcodegen_getModule(qrcode, module_x, module_y));
            uint32_t color = black ? (fg_color ? fg_color->color : 0xFF000000) : (bg_color ? bg_color->color : 0xFFFFFFFF);
            data[y * pitch + x] = color;
        }
    }

    // Return new Image userdata
    Image *img = (Image *)lua_newuserdata(L, sizeof(Image));
    img->tex = tex;

    luaL_getmetatable(L, "image");
    lua_setmetatable(L, -2);

    return 1;
}

/*static int lua_imagesave(lua_State *L){
    Image *image = (Image *)luaL_checkudata(L, 1, "image");
    const char *filename = luaL_checkstring(L, 2);
    const char *format = luaL_optstring(L, 3, "png");
    SceGxmTextureFormat tex_format = vita2d_texture_get_format(image->tex);
    int bpp = (tex_format == SCE_GXM_TEXTURE_FORMAT_U8U8U8_BGR) ? 3 : 4;
    unsigned int pitch = vita2d_texture_get_stride(image->tex) / bpp;
    unsigned int w = vita2d_texture_get_width(text->text);
	unsigned int h = vita2d_texture_get_height(text->text);
    int format_int = 0;

    if (format == "bmp") format_int = 1;
    else if (format == "png") format_int = 2;
    else if (format == "jpeg" || format == "jpg") format_int = 3;

    switch (format_int){
        default:
        case 1:
            SceUID fd = sceIoOpen(filename, SCE_O_CREAT|SCE_O_WRONLY|SCE_O_TRUNC, 0777);
		    uint8_t *bmp_content = (uint8_t*)malloc((w * h * 4)+0x36);
		    sceClibMemset(bmp_content, 0, 0x36);
		    *(uint16_t*)&bmp_content[0x0] = 0x4D42;
		    *(uint32_t*)&bmp_content[0x2] = (w * h * 4)+0x36;
		    *(uint32_t*)&bmp_content[0xA] = 0x36;
		    *(uint32_t*)&bmp_content[0xE] = 0x28;
		    *(uint32_t*)&bmp_content[0x12] = w;
		    *(uint32_t*)&bmp_content[0x16] = h;
		    *(uint32_t*)&bmp_content[0x1A] = 0x00200001;
		    *(uint32_t*)&bmp_content[0x22] = w * h * 4;
		    int x, y;
		    uint8_t* buffer = (uint8_t*)&bmp_content[0x36];
		    uint8_t* framebuf = (uint8_t*)vita2d_texture_get_datap(image->tex);
		    for (y = 0; y < h; y++) {
		    	for (x = 0; x < w; x++) {
		    		if (bpp == 3) {
		    			buffer[(x + y * w) * 4] = framebuf[(x+(h-y)*pitch)*bpp + 2];
		    			buffer[(x + y * w) * 4 + 1] = framebuf[(x+(h-y)*pitch)*bpp + 1];
		    			buffer[(x + y * w) * 4 + 2] = framebuf[(x+(h-y)*pitch)*bpp];
		    			buffer[(x + y * w) * 4 + 3] = 0xFF;
		    		} else {
		    			buffer[(x + y * w) * 4] = framebuf[(x+(h-y)*pitch)*bpp + 2];
		    			buffer[(x + y * w) * 4 + 1] = framebuf[(x+(h-y)*pitch)*bpp + 1];
		    			buffer[(x + y * w) * 4 + 2] = framebuf[(x+(h-y)*pitch)*bpp];
		    			buffer[(x + y * w) * 4 + 3] = framebuf[(x+(h-y)*pitch)*bpp + 3];
		    		}
		    	}
		    }
		    sceIoWrite(fd, bmp_content, (w * h * 4)+0x36);
		    free(bmp_content);
		    sceIoClose(fd);
            break;
        case 2: // png
            FILE *fh = fopen(filename, "wb");
		    uint8_t *raw_data = (uint8_t*)malloc(w * h * 4);
		    uint8_t* buffer = (uint8_t*)raw_data;
		    uint8_t* framebuf = (uint8_t*)vita2d_texture_get_datap(image->tex);
		    int x, y;
		    for (y = 0; y < h; y++) {
		    	for (x = 0; x < w; x++) {
		    		if (bpp == 3) {
		    			buffer[(x + y * w) * 4] = framebuf[(x+y*pitch)*bpp];
		    			buffer[(x + y * w) * 4 + 1] = framebuf[(x+y*pitch)*bpp + 1];
		    			buffer[(x + y * w) * 4 + 2] = framebuf[(x+y*pitch)*bpp + 2];
		    			buffer[(x + y * w) * 4 + 3] = 0xFF;
		    		} else {
		    			buffer[(x + y * w) * 4] = framebuf[(x+y*pitch)*bpp];
		    			buffer[(x + y * w) * 4 + 1] = framebuf[(x+y*pitch)*bpp + 1];
		    			buffer[(x + y * w) * 4 + 2] = framebuf[(x+y*pitch)*bpp + 2];
		    			buffer[(x + y * w) * 4 + 3] = framebuf[(x+y*pitch)*bpp + 3];
		    		}
		    	}
		    }
		    liq_attr *handle = liq_attr_create();
		    liq_image *input_image = liq_image_create_rgba(handle, raw_data, w, h, 0);
		    liq_result *res;
		    liq_image_quantize(input_image, handle, &res);
		    uint8_t *quant_raw = (uint8_t*)malloc(w * h);
		    liq_set_dithering_level(res, 1.0);
		    liq_write_remapped_image(res, input_image, quant_raw, w * h);
		    const liq_palette *palette = liq_get_palette(res);
		    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);;
		    png_infop info_ptr = png_create_info_struct(png_ptr);
		    setjmp(png_jmpbuf(png_ptr));
		    png_init_io(png_ptr, fh);
		    png_set_IHDR(png_ptr, info_ptr, w, h,
		    	8, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
		    	PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		    png_color *pal = (png_color*)png_malloc(png_ptr, palette->count*sizeof(png_color));
		    for (int i = 0; i < palette->count; i++) {
		    	png_color *col = &pal[i];
		    	col->red = palette->entries[i].r;
		    	col->green = palette->entries[i].g;
		    	col->blue = palette->entries[i].b;
		    }
		    png_set_PLTE(png_ptr, info_ptr, pal, palette->count);
		    png_write_info(png_ptr, info_ptr);
		    for (y = 0; y < h; y++) {
		    	png_write_row(png_ptr, &quant_raw[y * w]);
		    }
		    png_write_end(png_ptr, NULL);
		    fclose(fh);
		    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
		    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		    free(quant_raw);
		    free(raw_data);
		    liq_result_destroy(res);
		    liq_image_destroy(input_image);
		    liq_attr_destroy(handle);
            break;
        case 3:
            break;
    }
    return 0;
}*/

static int lua_imagedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	Color *color;
	if(argc <= 3){
		vita2d_draw_texture(image->tex, x, y);
	}else{
		color = lua_tocolor(L, 4);
		vita2d_draw_texture_tint(image->tex, x, y, color->color);
	}
	return 0;
}

static int lua_imagescaledraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float scalex = luaL_checknumber(L, 4);
	float scaley = luaL_checknumber(L, 5);
	Color *color;
	if(argc <= 5){
		vita2d_draw_texture_scale(image->tex, x, y, scalex, scaley);
	}else{
		color = lua_tocolor(L, 6);
		vita2d_draw_texture_tint_scale(image->tex, x, y, scalex, scaley, color->color);
	}
	return 0;
}

static int lua_imagerotatedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float radius = luaL_checknumber(L, 4);
	Color *color;
	if(argc <= 5){
		vita2d_draw_texture_rotate(image->tex, x, y, radius);
	}else{
		color = lua_tocolor(L, 5);
		vita2d_draw_texture_tint_rotate(image->tex, x, y, radius, color->color);
	}
	return 0;
}

static int lua_imagescalerotatedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
    float scalex = luaL_checknumber(L, 4);
	float scaley = luaL_checknumber(L, 5);
	float radius = luaL_checknumber(L, 6);
	Color *color;
	if(argc <= 5){
		vita2d_draw_texture_scale_rotate(image->tex, x, y, scalex, scaley, radius);
	}else{
		color = lua_tocolor(L, 5);
		vita2d_draw_texture_tint_scale_rotate(image->tex, x, y, scalex, scaley, radius, color->color);
	}
	return 0;
}

static int lua_imagepartdraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
    float tex_x = luaL_checknumber(L, 4);
    float tex_y = luaL_checknumber(L, 5);
    float tex_w = luaL_checknumber(L, 6);
    float tex_h = luaL_checknumber(L, 7);
	Color *color;
	if(argc <= 7){
		vita2d_draw_texture_part(image->tex, x, y, tex_x, tex_y, tex_w, tex_h);
	}else{
		color = lua_tocolor(L, 8);
		vita2d_draw_texture_tint_part(image->tex, x, y, tex_x, tex_y, tex_w, tex_h, color->color);
	}
	return 0;
}

static int lua_imagepartscaledraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
    float tex_x = luaL_checknumber(L, 4);
    float tex_y = luaL_checknumber(L, 5);
    float tex_w = luaL_checknumber(L, 6);
    float tex_h = luaL_checknumber(L, 7);
    float scalex = luaL_checknumber(L, 8);
	float scaley = luaL_checknumber(L, 9);
	Color *color;
	if(argc <= 9){
		vita2d_draw_texture_part_scale(image->tex, x, y, tex_x, tex_y, tex_w, tex_h, scalex, scaley);
	}else{
		color = lua_tocolor(L, 10);
		vita2d_draw_texture_tint_part_scale(image->tex, x, y, tex_x, tex_y, tex_w, tex_h, scalex, scaley, color->color);
	}
	return 0;
}

static int lua_imagepartscalerotatedraw(lua_State *L){
	int argc = lua_gettop(L);
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
    float tex_x = luaL_checknumber(L, 4);
    float tex_y = luaL_checknumber(L, 5);
    float tex_w = luaL_checknumber(L, 6);
    float tex_h = luaL_checknumber(L, 7);
    float scalex = luaL_checknumber(L, 8);
	float scaley = luaL_checknumber(L, 9);
    float rad = luaL_checknumber(L, 10);
	Color *color;
	if(argc <= 10){
		vita2d_draw_texture_part_scale_rotate(image->tex, x, y, tex_x, tex_y, tex_w, tex_h, scalex, scaley, rad);
	}else{
		color = lua_tocolor(L, 11);
		vita2d_draw_texture_part_tint_scale_rotate(image->tex, x, y, tex_x, tex_y, tex_w, tex_h, scalex, scaley, rad, color->color);
	}
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

static int lua_imagefilters(lua_State *L){
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	SceGxmTextureFilter min = luaL_checkinteger(L, 2);
	SceGxmTextureFilter mag = luaL_optinteger(L, 3, min);
	vita2d_texture_set_filters(image->tex, min, mag);
	return 0;
}

static int lua_imagemin(lua_State *L){
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	lua_pushinteger(L, vita2d_texture_get_min_filter(image->tex));
	return 1;
}

static int lua_imagemag(lua_State *L){
	Image *image = (Image *)luaL_checkudata(L, 1, "image");
	lua_pushinteger(L, vita2d_texture_get_mag_filter(image->tex));
	return 1;
}

static int lua_imagegc(lua_State *L) {
    Image *image = (Image *)luaL_checkudata(L, 1, "image");
    if (image->tex) {
        vita2d_free_texture(image->tex);
    }
    return 0;
}

static int lua_qrscan(lua_State *L) {
    Image *img = (Image *)luaL_checkudata(L, 1, "image");
    if (!img || !img->tex) {
        lua_pushnil(L);
        return 1;
    }

    int width = vita2d_texture_get_width(img->tex);
    int height = vita2d_texture_get_height(img->tex);
    uint32_t *pixels = vita2d_texture_get_datap(img->tex);

    struct quirc *qr = quirc_new();
    if (!qr) {
        lua_pushnil(L);
        return 1;
    }

    if (quirc_resize(qr, width, height) < 0) {
        quirc_destroy(qr);
        lua_pushnil(L);
        return 1;
    }

    uint8_t *gray = quirc_begin(qr, &width, &height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint32_t pixel = pixels[y * width + x];
            uint8_t r = (pixel >> 0) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;
            gray[y * width + x] = (r * 299 + g * 587 + b * 114) / 1000;
        }
    }

    quirc_end(qr);

    int num_codes = quirc_count(qr);
    if (num_codes <= 0) {
        quirc_destroy(qr);
        lua_pushnil(L);
        return 1;
    }

    struct quirc_code code;
    struct quirc_data data;

    quirc_extract(qr, 0, &code);
    quirc_decode_error_t err = quirc_decode(&code, &data);
    quirc_destroy(qr);

    if (err != QUIRC_SUCCESS) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, (const char *)data.payload);
    }

    return 1;
}

static int lua_grayscale(lua_State *L) {
    Image *img = (Image *)luaL_checkudata(L, 1, "image");

    int w = vita2d_texture_get_width(img->tex);
    int h = vita2d_texture_get_height(img->tex);
    uint32_t *pixels = vita2d_texture_get_datap(img->tex);

    for (int i = 0; i < w * h; i++) {
        uint32_t px = pixels[i];
        uint8_t r = (px >> 0) & 0xFF;
        uint8_t g = (px >> 8) & 0xFF;
        uint8_t b = (px >> 16) & 0xFF;
        uint8_t a = (px >> 24) & 0xFF;

        uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000;
        pixels[i] = (a << 24) | (gray << 16) | (gray << 8) | gray;
    }

    return 0;
}

static int lua_invert(lua_State *L) {
    Image *img = (Image *)luaL_checkudata(L, 1, "image");

    int w = vita2d_texture_get_width(img->tex);
    int h = vita2d_texture_get_height(img->tex);
    uint32_t *pixels = vita2d_texture_get_datap(img->tex);

    for (int i = 0; i < w * h; i++) {
        uint32_t px = pixels[i];
        uint8_t r = (px >> 0) & 0xFF;
        uint8_t g = (px >> 8) & 0xFF;
        uint8_t b = (px >> 16) & 0xFF;
        uint8_t a = (px >> 24) & 0xFF;

        pixels[i] = (a << 24) | ((255 - b) << 16) | ((255 - g) << 8) | (255 - r);
    }

    return 0;
}

static const luaL_Reg image_lib[] = {
    {"load", lua_imageload},
	{"new", lua_newimage},
	{"screen", lua_screenimage},
	{"qr", lua_qr},
    //{"save", lua_imagesave},
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"rotatedisplay", lua_imagerotatedraw},
    {"scalerotatedisplay", lua_imagescalerotatedraw},
    {"partdisplay", lua_imagepartdraw},
    {"scalepartdisplay", lua_imagepartscaledraw},
    {"scalerotatepartdisplay", lua_imagepartscalerotatedraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
	{"filter", lua_imagefilters},
	{"min", lua_imagemin},
	{"mag", lua_imagemag},
	{"qrscan", lua_qrscan},
	{"grayscale", lua_grayscale},
	{"invert", lua_invert},
    {NULL, NULL}
};

static const luaL_Reg image_methods[] = {
    {"display", lua_imagedraw},
	{"scaledisplay", lua_imagescaledraw},
	{"rotatedisplay", lua_imagerotatedraw},
    {"scalerotatedisplay", lua_imagescalerotatedraw},
    {"partdisplay", lua_imagepartdraw},
    {"scalepartdisplay", lua_imagepartscaledraw},
    {"scalerotatepartdisplay", lua_imagepartscalerotatedraw},
	{"width", lua_imagewidth},
	{"height", lua_imageheight},
	{"filter", lua_imagefilters},
	{"min", lua_imagemin},
	{"mag", lua_imagemag},
	{"qrscan", lua_qrscan},
	{"grayscale", lua_grayscale},
	{"invert", lua_invert},
	{"__gc", lua_imagegc},
    {NULL, NULL}
};

LUALIB_API int luaL_openimage(lua_State *L) {
	luaL_newmetatable(L, "image");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, image_methods);

	luaL_register(L, "image", image_lib);
	luaL_pushglobalint(L, SCE_GXM_TEXTURE_FILTER_POINT);
	luaL_pushglobalint(L, SCE_GXM_TEXTURE_FILTER_LINEAR);
	luaL_pushglobalint(L, SCE_GXM_TEXTURE_FILTER_MIPMAP_LINEAR);
	luaL_pushglobalint(L, SCE_GXM_TEXTURE_FILTER_MIPMAP_POINT);
	luaL_pushglobalint(L, qrcodegen_Ecc_LOW);
	luaL_pushglobalint(L, qrcodegen_Ecc_MEDIUM);
	luaL_pushglobalint(L, qrcodegen_Ecc_QUARTILE);
	luaL_pushglobalint(L, qrcodegen_Ecc_HIGH);
	luaL_pushglobalint(L, qrcodegen_Mask_AUTO);
	luaL_pushglobalint(L, qrcodegen_Mask_0);
	luaL_pushglobalint(L, qrcodegen_Mask_1);
	luaL_pushglobalint(L, qrcodegen_Mask_2);
	luaL_pushglobalint(L, qrcodegen_Mask_3);
	luaL_pushglobalint(L, qrcodegen_Mask_4);
	luaL_pushglobalint(L, qrcodegen_Mask_5);
	luaL_pushglobalint(L, qrcodegen_Mask_6);
	luaL_pushglobalint(L, qrcodegen_Mask_7);
	luaL_pushglobalint(L, qrcodegen_VERSION_MIN);
	luaL_pushglobalint(L, qrcodegen_VERSION_MAX);
    return 1;
}