/*
    LifeLua WIP
    Audio library
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
#include <mpg123.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#define DR_WAV_IMPLEMENTATION
#include "include/dr_wav.h"

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/vitaaudiolib.h"

#include "lj_lifeinit.h"

// For MP3 ID3 tags
struct genre {
	int code;
	char text[112];
};

// For MP3 ID3 tags
struct genre genreList[] = {
	{0 , "Blues"}, {1 , "Classic Rock"}, {2 , "Country"}, {3 , "Dance"}, {4 , "Disco"}, {5 , "Funk"}, {6 , "Grunge"}, {7 , "Hip-Hop"}, {8 , "Jazz"}, {9 , "Metal"}, {10 , "New Age"},
	{11 , "Oldies"}, {12 , "Other"}, {13 , "Pop"}, {14 , "R&B"}, {15 , "Rap"}, {16 , "Reggae"}, {17 , "Rock"}, {18 , "Techno"}, {19 , "Industrial"}, {20 , "Alternative"},
	{21 , "Ska"}, {22 , "Death Metal"}, {23 , "Pranks"}, {24 , "Soundtrack"}, {25 , "Euro-Techno"}, {26 , "Ambient"}, {27 , "Trip-Hop"}, {28 , "Vocal"}, {29 , "Jazz+Funk"}, {30 , "Fusion"},
	{31 , "Trance"}, {32 , "Classical"}, {33 , "Instrumental"}, {34 , "Acid"}, {35 , "House"}, {36 , "Game"}, {37 , "Sound Clip"}, {38 , "Gospel"}, {39 , "Noise"}, {40 , "Alternative Rock"},
	{41 , "Bass"}, {42 , "Soul"}, {43 , "Punk"}, {44 , "Space"}, {45 , "Meditative"}, {46 , "Instrumental Pop"}, {47 , "Instrumental Rock"}, {48 , "Ethnic"}, {49 , "Gothic"}, {50 , "Darkwave"},
	{51 , "Techno-Industrial"}, {52 , "Electronic"}, {53 , "Pop-Folk"}, {54 , "Eurodance"}, {55 , "Dream"}, {56 , "Southern Rock"}, {57 , "Comedy"}, {58 , "Cult"}, {59 , "Gangsta"}, {60 , "Top 40"},
	{61 , "Christian Rap"}, {62 , "Pop/Funk"}, {63 , "Jungle"}, {64 , "Native US"}, {65 , "Cabaret"}, {66 , "New Wave"}, {67 , "Psychadelic"}, {68 , "Rave"}, {69 , "Showtunes"}, {70 , "Trailer"},
	{71 , "Lo-Fi"}, {72 , "Tribal"}, {73 , "Acid Punk"}, {74 , "Acid Jazz"}, {75 , "Polka"}, {76 , "Retro"}, {77 , "Musical"}, {78 , "Rock & Roll"}, {79 , "Hard Rock"}, {80 , "Folk"},
	{81 , "Folk-Rock"}, {82 , "National Folk"}, {83 , "Swing"}, {84 , "Fast Fusion"}, {85 , "Bebob"}, {86 , "Latin"}, {87 , "Revival"}, {88 , "Celtic"}, {89 , "Bluegrass"}, {90 , "Avantgarde"},
	{91 , "Gothic Rock"}, {92 , "Progressive Rock"}, {93 , "Psychedelic Rock"}, {94 , "Symphonic Rock"}, {95 , "Slow Rock"}, {96 , "Big Band"}, {97 , "Chorus"}, {98 , "Easy Listening"}, {99 , "Acoustic"},
	{100 , "Humour"}, {101 , "Speech"}, {102 , "Chanson"}, {103 , "Opera"}, {104 , "Chamber Music"}, {105 , "Sonata"}, {106 , "Symphony"}, {107 , "Booty Bass"}, {108 , "Primus"}, {109 , "Porn Groove"},
	{110 , "Satire"}, {111 , "Slow Jam"}, {112 , "Club"}, {113 , "Tango"}, {114 , "Samba"}, {115 , "Folklore"}, {116 , "Ballad"}, {117 , "Power Ballad"}, {118 , "Rhytmic Soul"}, {119 , "Freestyle"}, {120 , "Duet"},
	{121 , "Punk Rock"}, {122 , "Drum Solo"}, {123 , "A capella"}, {124 , "Euro-House"}, {125 , "Dance Hall"}, {126 , "Goa"}, {127 , "Drum & Bass"}, {128 , "Club-House"}, {129 , "Hardcore"}, {130 , "Terror"},
	{131 , "Indie"}, {132 , "BritPop"}, {133 , "Negerpunk"}, {134 , "Polsk Punk"}, {135 , "Beat"}, {136 , "Christian Gangsta"}, {137 , "Heavy Metal"}, {138 , "Black Metal"}, {139 , "Crossover"}, {140 , "Contemporary C"},
	{141 , "Christian Rock"}, {142 , "Merengue"}, {143 , "Salsa"}, {144 , "Thrash Metal"}, {145 , "Anime"}, {146 , "JPop"}, {147 , "SynthPop"}
};

typedef enum {
    AUDIO_TYPE_RAW,
    AUDIO_TYPE_MP3,
    AUDIO_TYPE_WAV
} AudioType;

typedef struct {
    AudioType type;
    bool loop;
    int channel;
    bool paused;
    bool playing;
    uint64_t frames_played;
    union {
        struct {
            FILE *file;
        } raw;
        struct {
            mpg123_handle *handle;
            mpg123_id3v1 *v1;
            mpg123_id3v2 *v2;
            off_t start_frame;
        } mp3;
        struct {
            drwav wav;
            size_t framePos;
        } wav;
    };
} Audio;

bool audio_active = false;

static void audio_callback(void *stream, unsigned int length, void *userdata) {
    Audio *aud = (Audio *)userdata;
    if (!aud) return;

    unsigned int bytes_needed = length * 4; // stereo 16-bit = 4 bytes/sample
    unsigned int bytes_filled = 0;

    aud->playing = true;
    while (bytes_filled < bytes_needed) {
        size_t done = 0;
        unsigned char *dst = ((unsigned char*)stream) + bytes_filled;
        
        switch (aud->type){
            case AUDIO_TYPE_RAW: {
                size_t read = fread(dst, 1, bytes_needed - bytes_filled, aud->raw.file);
                bytes_filled += read;
                aud->frames_played += read;

                if (read == 0 && aud->loop) {
                    fseek(aud->raw.file, 0, SEEK_SET);
                } else if (read == 0) {
                    memset(dst, 0, bytes_needed - bytes_filled);
                    break;
                }
            }
            case AUDIO_TYPE_MP3: {
                int err = mpg123_read(aud->mp3.handle, dst, bytes_needed - bytes_filled, &done);
                bytes_filled += done;
                aud->frames_played += done;

                if (err == MPG123_DONE && aud->loop) {
                    mpg123_seek(aud->mp3.handle, aud->mp3.start_frame, SEEK_SET);
                } else if (err == MPG123_DONE) {
                    memset(dst, 0, bytes_needed - bytes_filled);
                    break;
                }
            }
            case AUDIO_TYPE_WAV: {
                drwav_uint64 frames_to_read = (bytes_needed - bytes_filled) / (aud->wav.wav.channels * sizeof(int16_t));
                drwav_uint64 frames_read = drwav_read_pcm_frames_s16(&aud->wav.wav, frames_to_read, (drwav_int16 *)(dst + bytes_filled));
            
                size_t bytes_just_read = frames_read * aud->wav.wav.channels * sizeof(int16_t);
                bytes_filled += bytes_just_read;
                aud->frames_played += frames_read;
            
                if (frames_read < frames_to_read) {
                    if (aud->loop) {
                        drwav_seek_to_pcm_frame(&aud->wav.wav, 0);
                    } else {
                        memset(dst + bytes_filled, 0, bytes_needed - bytes_filled);
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
        //if(aud->paused) sceKernelDelayThreadCB(100);
    }
    aud->playing = false;
}

static int lua_audioload(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);

    Audio *aud = (Audio *)lua_newuserdata(L, sizeof(Audio));
    memset(aud, 0, sizeof(Audio));
    aud->channel = 0;

    if(string_ends_with(path, ".mp3")){
        if (mpg123_init() != MPG123_OK) return luaL_error(L, "Failed to init mpg123");

        mpg123_handle *mh = mpg123_new(NULL, NULL);
        if (!mh) return luaL_error(L, "Failed to create mpg123 handle");

        mpg123_param(mh, MPG123_FLAGS, MPG123_FORCE_SEEKABLE | MPG123_FUZZY | MPG123_SEEKBUFFER | MPG123_GAPLESS, 0.0);

        mpg123_param(mh, MPG123_INDEX_SIZE, -1, 0.0);

        mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.0);

        if (mpg123_open(mh, path) != MPG123_OK) return luaL_error(L, "Failed to open MP3");

        mpg123_seek(mh, 0, SEEK_SET);
        if(mpg123_meta_check(mh) & MPG123_ID3) mpg123_id3(mh, &aud->mp3.v1, &aud->mp3.v2);

        long rate;
        int channels, encoding;
        if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) return luaL_error(L, "Unsupported MP3 format");

        aud->type = AUDIO_TYPE_MP3;
        aud->mp3.handle = mh;
        aud->mp3.start_frame = mpg123_tell(mh);
        aud->paused = false;
    }else if(string_ends_with(path, ".wav")){
        if (!drwav_init_file(&aud->wav.wav, path, NULL)) {
            return luaL_error(L, "Failed to load WAV file: %s", path);
        }

        aud->type = AUDIO_TYPE_WAV;
        aud->wav.framePos = 0;
    }else{
        FILE *f = fopen(path, "rb");
        if (!f) return luaL_error(L, "Failed to open audio file");

        aud->type = AUDIO_TYPE_RAW;
        aud->raw.file = f;
    }

    luaL_getmetatable(L, "audio");
    lua_setmetatable(L, -2);
    return 1;
}

static int lua_audioplay(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    int loop = lua_toboolean(L, 2);
    aud->loop = loop;
    if(!audio_active){
        if(aud->type == AUDIO_TYPE_MP3 && aud->mp3.handle){
            long rate;
            int channels, encoding;
            mpg123_getformat(aud->mp3.handle, &rate, &channels, &encoding);
            vitaAudioInit(rate, (channels == 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_WAV){
            vitaAudioInit(aud->wav.wav.sampleRate, (aud->wav.wav.channels == 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else{
            vitaAudioInit(48000, SCE_AUDIO_OUT_MODE_STEREO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }

        audio_active = true;
    }

    vitaAudioSetChannelCallback(aud->channel, audio_callback, aud);
    return 0;
}

static int lua_audiostop(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    vitaAudioSetChannelCallback(aud->channel, NULL, NULL);
    vitaAudioEndPre();
	vitaAudioEnd();
    audio_active = false;
    return 0;
}

static int lua_audiopause(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    aud->paused = lua_toboolean(L, 1);
    return 0;
}

static int lua_audioplaying(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    lua_pushboolean(L, aud->playing);
    return 1;
}

/*static int lua_audioduration(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");

    if (aud->type == AUDIO_TYPE_MP3) {
        if (aud->mp3.sample_rate > 0 && aud->total_frames > 0)
            lua_pushnumber(L, (double)aud->total_frames / aud->mp3.sample_rate);
        else
            lua_pushnil(L);  // unknown duration
    } else if (aud->type == AUDIO_TYPE_WAV) {
        lua_pushnumber(L, (double)aud->total_frames / aud->wav.sample_rate);
    } else {
        lua_pushnil(L);
    }

    return 1;
}

static int lua_audioremaining(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    if(aud->type == AUDIO_TYPE_MP3 && aud->mp3.handle) {
        long rate;
        mpg123_getformat(aud->mp3.handle, &rate, NULL, NULL);
        lua_pushnumber(L, (aud->mp3.start_frame / rate));
    }
    else lua_pushnil(L);
    return 1;
}*/

static int lua_audiogc(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    if (aud->type == AUDIO_TYPE_RAW && aud->raw.file) fclose(aud->raw.file);
    else if (aud->type == AUDIO_TYPE_MP3 && aud->mp3.handle) mpg123_close(aud->mp3.handle), mpg123_delete(aud->mp3.handle);
    else if (aud->type == AUDIO_TYPE_WAV) drwav_uninit(&aud->wav.wav);
    return 0;
}

static int lua_id3v1(lua_State *L){
    Audio *audio = (Audio *)luaL_checkudata(L, 1, "audio");
    if(audio->mp3.v1 != NULL){
        lua_newtable(L);
        lua_pushstring(L, audio->mp3.v1->title); lua_setfield(L, -2, "title");
        lua_pushstring(L, audio->mp3.v1->artist); lua_setfield(L, -2, "artist");
        lua_pushstring(L, audio->mp3.v1->album); lua_setfield(L, -2, "album");
        lua_pushstring(L, audio->mp3.v1->comment); lua_setfield(L, -2, "comment");
        lua_pushstring(L, audio->mp3.v1->year); lua_setfield(L, -2, "year");
        lua_pushstring(L, audio->mp3.v1->tag); lua_setfield(L, -2, "tag");
        lua_pushstring(L, genreList[audio->mp3.v1->genre].text); lua_setfield(L, -2, "genre");
    }else lua_pushnil(L);
    return 1;
}

static int lua_id3v2(lua_State *L){
    Audio *audio = (Audio *)luaL_checkudata(L, 1, "audio");
    if(audio->mp3.v2 != NULL){
        lua_newtable(L);
        lua_pushstring(L, audio->mp3.v2->title->p); lua_setfield(L, -2, "title");
        lua_pushstring(L, audio->mp3.v2->artist->p); lua_setfield(L, -2, "artist");
        lua_pushstring(L, audio->mp3.v2->album->p); lua_setfield(L, -2, "album");
        lua_pushstring(L, audio->mp3.v2->comment->p); lua_setfield(L, -2, "comment");
        lua_pushstring(L, audio->mp3.v2->year->p); lua_setfield(L, -2, "year");
        lua_pushstring(L, audio->mp3.v2->genre->p); lua_setfield(L, -2, "genre");
        Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
        if(!strcmp(audio->mp3.v2->picture->mime_type.p, "image/jpg") || !strcmp(audio->mp3.v2->picture->mime_type.p, "image/jpeg")) image->tex = vita2d_load_JPEG_buffer(audio->mp3.v2->picture->data, audio->mp3.v2->picture->size);
        else if(!strcmp(audio->mp3.v2->picture->mime_type.p, "image/png")) image->tex = vita2d_load_PNG_buffer(audio->mp3.v2->picture->data);
        if(!image->tex) lua_pushnil(L);
        luaL_getmetatable(L, "image");
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "cover");
    }else lua_pushnil(L);
    return 1;
}

static const luaL_Reg audio_lib[] = {
    {"load", lua_audioload},
    {"play", lua_audioplay},
    {"pause", lua_audiopause},
    {"playing", lua_audioplaying},
    //{"duration", lua_audioduration},
    //{"remaining", lua_audioremaining},
    {"id3v1", lua_id3v1},
    {"id3v2", lua_id3v2},
    {"stop", lua_audiostop},
    {NULL, NULL}
};

static const luaL_Reg audio_methods[] = {
    {"play", lua_audioplay},
    {"pause", lua_audiopause},
    {"playing", lua_audioplaying},
    //{"duration", lua_audioduration},
    //{"remaining", lua_audioremaining},
    {"id3v1", lua_id3v1},
    {"id3v2", lua_id3v2},
    {"stop", lua_audiostop},
    {"__gc", lua_audiogc},
    {NULL, NULL}
};

LUALIB_API int luaL_openaudio(lua_State *L) {
	luaL_newmetatable(L, "audio");
	lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, audio_methods);

	luaL_register(L, "audio", audio_lib);
    return 1;
}