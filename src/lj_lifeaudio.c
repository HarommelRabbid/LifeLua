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
#include <FLAC/stream_decoder.h>
#include <FLAC/metadata.h>
#include <opus/opusfile.h>
#include <xmp.h>

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
    AUDIO_TYPE_WAV,
    AUDIO_TYPE_OGG,
    AUDIO_TYPE_FLAC,
    AUDIO_TYPE_OPUS,
    AUDIO_TYPE_XM,
    AUDIO_TYPE_AT9,
    AUDIO_TYPE_AT3
} AudioType;

typedef struct {
    AudioType type;
    bool loop;
    volatile bool paused;
    bool playing;
    int channel;
    SceKernelLwMutexWork mutex;
    SceKernelLwCondWork cond;
    union {
        struct {
            FILE *file;
            uint64_t frames_played;
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
            drwav_uint64 frames_played;
        } wav;
        struct {
            OggVorbis_File ogg;
            vorbis_info *info;
            ogg_int64_t total_frames;
        } ogg;
        struct {
            FLAC__StreamDecoder *decoder;
            bool metadata_parsed;
            FLAC__uint32 sampleRate;
            FLAC__uint8 channels;
            FLAC__uint8 bps;
            FLAC__uint64 totalSamples;
            FLAC__uint64 playedSamples;
            FLAC__int16 *decode_buffer;
            size_t decode_buffer_capacity;
            size_t decode_buffer_offset;
            size_t decode_buffer_samples;
        } flac;
        struct {
            OggOpusFile *opus;
        } opus;
        struct {
            xmp_context ctx;
            struct xmp_frame_info fi;
            struct xmp_module_info mi;
        } xmp;
    };
} Audio;

volatile bool audio_active = false;

static void flac_metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
    Audio *aud = (Audio *)client_data;
    if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        aud->flac.sampleRate = metadata->data.stream_info.sample_rate;
        aud->flac.channels = metadata->data.stream_info.channels;
        aud->flac.bps = metadata->data.stream_info.bits_per_sample;
        aud->flac.totalSamples = metadata->data.stream_info.total_samples;
        aud->flac.metadata_parsed = true;
    }
}

static FLAC__StreamDecoderWriteStatus flac_write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
    Audio *aud = (Audio *)client_data;
    const size_t samples_in_frame = frame->header.blocksize;
    const size_t total_interleaved_samples = samples_in_frame * aud->flac.channels;

    // Ensure our decode buffer is large enough for the entire frame
    if (total_interleaved_samples > aud->flac.decode_buffer_capacity) {
        free(aud->flac.decode_buffer);
        aud->flac.decode_buffer = malloc(total_interleaved_samples * sizeof(FLAC__int16));
        if (!aud->flac.decode_buffer) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
        aud->flac.decode_buffer_capacity = total_interleaved_samples;
    }

    // Reset buffer state for the new frame
    aud->flac.decode_buffer_offset = 0;
    aud->flac.decode_buffer_samples = samples_in_frame;

    FLAC__int16 *out_ptr = aud->flac.decode_buffer;

    // Interleave and convert the full frame into our intermediate buffer
    for (size_t i = 0; i < samples_in_frame; i++) {
        for (FLAC__uint8 c = 0; c < aud->flac.channels; c++) {
            FLAC__int32 pSample = buffer[c][i];
            // Scale sample to 16-bit
            if (aud->flac.bps > 16) {
                *out_ptr++ = (FLAC__int16)(pSample >> (aud->flac.bps - 16));
            } else {
                *out_ptr++ = (FLAC__int16)(pSample << (16 - aud->flac.bps));
            }
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void flac_error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
    //sceClibPrintf("FLAC decoding error: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

static void audio_callback(void *stream, unsigned int length, void *userdata){
    Audio *aud = (Audio *)userdata;
    if (!aud || !audio_active) return;

    sceKernelLockLwMutex(&aud->mutex, 1, NULL);
    while (aud->paused) {
        sceKernelWaitLwCond(&aud->cond, NULL);
    }
    sceKernelUnlockLwMutex(&aud->mutex, 1);

    int channels;
    switch (aud->type){
        case AUDIO_TYPE_MP3:
            mpg123_getformat(aud->mp3.handle, NULL, &channels, NULL);
            break;
        case AUDIO_TYPE_WAV:
            channels = aud->wav.wav.channels;
            break;
        case AUDIO_TYPE_OGG:
            channels = aud->ogg.info->channels;
            break;
        case AUDIO_TYPE_FLAC:
            channels = aud->flac.channels;
            break;
        case AUDIO_TYPE_OPUS:
            channels = op_channel_count(aud->opus.opus, -1);
            break;
        default:
            channels = 2;
            break;
    }

    unsigned int bytes_needed = length * channels * 2;
    unsigned int bytes_filled = 0;

    while (bytes_filled < bytes_needed) {
        aud->playing = true;
        size_t done = 0;
        unsigned char *dst = ((unsigned char*)stream) + bytes_filled;
        bool stream_done = false;

        switch (aud->type) {
            case AUDIO_TYPE_RAW: {
                size_t read = fread(dst, 1, bytes_needed - bytes_filled, aud->raw.file);
                bytes_filled += read;
                aud->raw.frames_played += read;

                if (read == 0) {
                    if (aud->loop) {
                        fseek(aud->raw.file, 0, SEEK_SET);
                        aud->raw.frames_played = 0;
                    } else {
                        stream_done = true;
                    }
                }
                break;
            }
            case AUDIO_TYPE_MP3: {
                int err = mpg123_read(aud->mp3.handle, dst, bytes_needed - bytes_filled, &done);
                bytes_filled += done;

                if (err == MPG123_DONE) {
                    if (aud->loop) {
                        mpg123_seek(aud->mp3.handle, aud->mp3.start_frame, SEEK_SET);
                    } else {
                        stream_done = true;
                    }
                }
                break;
            }
            case AUDIO_TYPE_WAV: {
                int bytes_per_frame = aud->wav.wav.channels * sizeof(drwav_int16);
                int frames_to_read = (bytes_needed - bytes_filled) / bytes_per_frame;

                drwav_uint64 frames_read = drwav_read_pcm_frames_s16(&aud->wav.wav, (drwav_uint64)frames_to_read, (drwav_int16 *)dst);
                int bytes_read = frames_read * bytes_per_frame;
                bytes_filled += bytes_read;
                aud->wav.frames_played += bytes_read;

                if (frames_read == 0) {
                    if (aud->loop) {
                        drwav_seek_to_pcm_frame(&aud->wav.wav, 0);
                        aud->wav.frames_played = 0;
                    } else {
                        stream_done = true;
                    }
                }
                break;
            }
            case AUDIO_TYPE_OGG: {
                int current_section;
                long ret = ov_read(&aud->ogg.ogg, (char *)dst, bytes_needed - bytes_filled, 0, 2, 1, &current_section);
                if (ret == 0) {
                    if (aud->loop) {
                        ov_raw_seek(&aud->ogg.ogg, 0);
                    } else {
                        stream_done = true;
                    }
                } else if (ret < 0) {
                    stream_done = true;
                } else {
                    bytes_filled += ret;
                }
                break;
            }
            case AUDIO_TYPE_FLAC: {
                size_t bytes_per_sample_frame = aud->flac.channels * sizeof(FLAC__int16);
                size_t samples_to_fill = (bytes_needed - bytes_filled) / bytes_per_sample_frame;
                size_t samples_filled = 0;

                while (samples_filled < samples_to_fill) {
                    size_t samples_in_buffer = aud->flac.decode_buffer_samples - aud->flac.decode_buffer_offset;

                    // If our buffer is empty, decode a new frame.
                    if (samples_in_buffer == 0) {
                        if (FLAC__stream_decoder_get_state(aud->flac.decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
                            if (aud->loop) {
                                FLAC__stream_decoder_seek_absolute(aud->flac.decoder, 0);
                            } else {
                                stream_done = true;
                                break;
                            }
                        }
                        
                        // This calls flac_write_callback, which will fill our intermediate buffer
                        if (!FLAC__stream_decoder_process_single(aud->flac.decoder)) {
                            stream_done = true;
                            break; 
                        }
                        // Loop again to re-check samples_in_buffer with the new data
                        continue;
                    }

                    size_t samples_to_copy = samples_to_fill - samples_filled;
                    if (samples_to_copy > samples_in_buffer) {
                        samples_to_copy = samples_in_buffer;
                    }

                    // Copy from our buffer to the hardware stream buffer
                    memcpy(dst, 
                           aud->flac.decode_buffer + (aud->flac.decode_buffer_offset * aud->flac.channels),
                           samples_to_copy * bytes_per_sample_frame);

                    dst += samples_to_copy * bytes_per_sample_frame;
                    aud->flac.decode_buffer_offset += samples_to_copy;
                    samples_filled += samples_to_copy;
                    aud->flac.playedSamples += samples_to_copy;
                }

                bytes_filled += samples_filled * bytes_per_sample_frame;
                break;
            }
            case AUDIO_TYPE_OPUS: {
                int current_section;
                long ret = op_read(aud->opus.opus, (opus_int16 *)dst, (bytes_needed - bytes_filled) / 2, &current_section);
                if (ret == 0) {
                    if (aud->loop) {
                        op_raw_seek(aud->opus.opus, 0);
                    } else {
                        stream_done = true;
                    }
                } else if (ret < 0) {
                    stream_done = true;
                } else {
                    int bytes_read = ret * channels * sizeof(opus_int16);
                    bytes_filled += bytes_read;
                }
                break;
            }
            case AUDIO_TYPE_XM: {
                int ret = xmp_play_buffer(aud->xmp.ctx, dst, bytes_needed - bytes_filled, !aud->loop); //loop inversed because 0 is loop and 1 is no loop
                if (ret == 0) {
                    bytes_filled += (bytes_needed - bytes_filled);
                } else {
                    stream_done = true;
                }
                break;
            }
            default:
                stream_done = true;
                break;
        }

        if (stream_done) {
            // Fill the rest of the buffer with silence
            memset(((unsigned char*)stream) + bytes_filled, 0, bytes_needed - bytes_filled);
            aud->playing = false;
            vitaAudioSetChannelCallback(aud->channel, NULL, NULL);
            break;
        }
    }
}

static int lua_audioload(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);

    Audio *aud = (Audio *)lua_newuserdata(L, sizeof(Audio));
    memset(aud, 0, sizeof(Audio));
    aud->channel = 0;
    sceKernelCreateLwMutex(&aud->mutex, "LifeLua Audio Mutex", 0, 0, NULL);
    sceKernelCreateLwCond(&aud->cond, "LifeLua Audio Condition", 0, &aud->mutex, NULL);
    aud->paused = false;

    if(string_ends_with(path, ".mp3")){
        mpg123_init();

        mpg123_handle *mh = mpg123_new(NULL, NULL);

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
        if (!drwav_init_file(&aud->wav.wav, path, NULL)) return luaL_error(L, "Failed to load WAV file: %s", path);

        aud->type = AUDIO_TYPE_WAV;
        aud->wav.framePos = 0;
        aud->wav.frames_played = 0;
    }else if (string_ends_with(path, ".ogg")) {
        if (ov_fopen(path, &aud->ogg.ogg) < 0) return luaL_error(L, "Failed to open OGG file");

        aud->ogg.info = ov_info(&aud->ogg.ogg, -1);
        aud->ogg.total_frames = ov_pcm_total(&aud->ogg.ogg, -1);
        aud->type = AUDIO_TYPE_OGG;
    }else if (string_ends_with(path, ".flac")) {
        aud->type = AUDIO_TYPE_FLAC;
        aud->flac.decoder = FLAC__stream_decoder_new();
        if (aud->flac.decoder == NULL) {
            return luaL_error(L, "Failed to create FLAC decoder instance");
        }

        aud->flac.metadata_parsed = false;
        
        FLAC__stream_decoder_set_md5_checking(aud->flac.decoder, false);
        FLAC__stream_decoder_set_metadata_ignore_all(aud->flac.decoder);
        FLAC__stream_decoder_set_metadata_respond(aud->flac.decoder, FLAC__METADATA_TYPE_STREAMINFO);
        
        FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_file(
            aud->flac.decoder, path, flac_write_callback, 
            flac_metadata_callback, flac_error_callback, aud
        );

        if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
            FLAC__stream_decoder_delete(aud->flac.decoder);
            aud->flac.decoder = NULL;
            return luaL_error(L, "Failed to initialize FLAC decoder: %s", FLAC__StreamDecoderInitStatusString[init_status]);
        }

        if (!FLAC__stream_decoder_process_until_end_of_metadata(aud->flac.decoder)) {
            FLAC__stream_decoder_finish(aud->flac.decoder);
            FLAC__stream_decoder_delete(aud->flac.decoder);
            aud->flac.decoder = NULL;
            return luaL_error(L, "Failed to process .FLAC metadata");
        }
    
        aud->flac.decode_buffer = NULL;
        aud->flac.decode_buffer_capacity = 0;
        aud->flac.decode_buffer_offset = 0;
        aud->flac.decode_buffer_samples = 0;
        aud->flac.playedSamples = 0;

        if (!aud->flac.metadata_parsed) {
            FLAC__stream_decoder_finish(aud->flac.decoder);
            FLAC__stream_decoder_delete(aud->flac.decoder);
            aud->flac.decoder = NULL;
            return luaL_error(L, "STREAMINFO block not found in .FLAC");
        }
    }else if (string_ends_with(path, ".opus")) {
        if ((aud->opus.opus = op_open_file(path, NULL)) == NULL) return luaL_error(L, "Failed to load OPUS file: %s", path);

        aud->type = AUDIO_TYPE_OPUS;
    }else{
        aud->xmp.ctx = xmp_create_context();
        if (xmp_load_module(aud->xmp.ctx, path) < 0){
            xmp_free_context(aud->xmp.ctx);
            FILE *f = fopen(path, "rb");
            if (!f) return luaL_error(L, "Failed to open audio file");

            aud->type = AUDIO_TYPE_RAW;
            aud->raw.file = f;
            aud->raw.frames_played = 0;
        }else{
            xmp_get_module_info(aud->xmp.ctx, &aud->xmp.mi);
            aud->type = AUDIO_TYPE_XM;
        }
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
            vitaAudioInit(rate, (channels >= 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_WAV){
            vitaAudioInit(aud->wav.wav.sampleRate, (aud->wav.wav.channels >= 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_OGG){
            vitaAudioInit(aud->ogg.info->rate, (aud->ogg.info->channels >= 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_FLAC){
            vitaAudioInit(aud->flac.sampleRate, (aud->flac.channels >= 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_OPUS){
            // Opus always decodes at 48000 kHz
            vitaAudioInit(48000, (op_channel_count(aud->opus.opus, -1) >= 2) ? SCE_AUDIO_OUT_MODE_STEREO : SCE_AUDIO_OUT_MODE_MONO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else if(aud->type == AUDIO_TYPE_XM){
            xmp_start_player(aud->xmp.ctx, 48000, 0);
            vitaAudioInit(48000, SCE_AUDIO_OUT_MODE_STEREO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }else{
            vitaAudioInit(48000, SCE_AUDIO_OUT_MODE_STEREO);
            vitaAudioSetVolume(0, SCE_AUDIO_OUT_MAX_VOL, SCE_AUDIO_OUT_MAX_VOL);
        }

        audio_active = true;
    }

    if (!aud->playing) {
        switch (aud->type) {
            case AUDIO_TYPE_RAW:
                fseek(aud->raw.file, 0, SEEK_SET);
                aud->raw.frames_played = 0;
                break;
            case AUDIO_TYPE_MP3:
                mpg123_seek(aud->mp3.handle, aud->mp3.start_frame, SEEK_SET);
                break;
            case AUDIO_TYPE_WAV:
                drwav_seek_to_pcm_frame(&aud->wav.wav, 0);
                aud->wav.frames_played = 0;
                break;
            case AUDIO_TYPE_FLAC:
                FLAC__stream_decoder_seek_absolute(aud->flac.decoder, 0);
                aud->flac.playedSamples = 0;
                break;
            case AUDIO_TYPE_OGG:
                ov_raw_seek(&aud->ogg.ogg, 0);
                break;
            case AUDIO_TYPE_OPUS:
                op_raw_seek(aud->opus.opus, 0);
                break;
            case AUDIO_TYPE_XM:
                xmp_seek_time(aud->xmp.ctx, 0);
                break;
            default: break;
        }
    }

    vitaAudioSetChannelCallback(aud->channel, audio_callback, aud);
    return 0;
}

static int lua_audioseek(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    double seconds = luaL_checknumber(L, 2);

    switch (aud->type) {
        case AUDIO_TYPE_RAW: {
            fseek(aud->raw.file, seconds * 48000, SEEK_SET); // hardcoded sample rate unfortunately
            aud->raw.frames_played = seconds * 48000;
            break;
        }
        case AUDIO_TYPE_MP3: {
            long rate;
            mpg123_getformat(aud->mp3.handle, &rate, NULL, NULL);
            mpg123_seek(aud->mp3.handle, seconds * rate, SEEK_SET);
            break;
        }
        case AUDIO_TYPE_WAV: {
            drwav_seek_to_pcm_frame(&aud->wav.wav, (drwav_uint64)seconds * aud->wav.wav.sampleRate);
            aud->wav.frames_played = (drwav_uint64)seconds * aud->wav.wav.sampleRate;
            break;
        }
        case AUDIO_TYPE_FLAC: {
            uint64_t target_sample = (uint64_t)(seconds * aud->flac.sampleRate);
            if (target_sample > aud->flac.totalSamples) {
                target_sample = aud->flac.totalSamples;
            }
            FLAC__stream_decoder_seek_absolute(aud->flac.decoder, target_sample);
            aud->flac.playedSamples = target_sample;
            break;
        }
        case AUDIO_TYPE_OGG: {
            ov_time_seek(&aud->ogg.ogg, seconds);
            break;
        }
        case AUDIO_TYPE_OPUS: {
            op_raw_seek(aud->opus.opus, (opus_int64)seconds * 48000);
            break;
        }
        case AUDIO_TYPE_XM: {
            xmp_seek_time(aud->xmp.ctx, (int)(seconds * 1000));
            break;
        }
        default: break;
    }

    return 0;
}

static int lua_audiostop(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    if(aud->type == AUDIO_TYPE_XM) xmp_end_player(aud->xmp.ctx); 
    if(audio_active){
        vitaAudioSetChannelCallback(aud->channel, NULL, NULL);
        vitaAudioEndPre();
        vitaAudioEnd();
        audio_active = false;
    }
    return 0;
}

static int lua_audiopause(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    bool paused = lua_toboolean(L, 2);
    if(paused) aud->paused = true;
    else{
        sceKernelLockLwMutex(&aud->mutex, 1, NULL);
        aud->paused = false;
        sceKernelSignalLwCond(&aud->cond);
        sceKernelUnlockLwMutex(&aud->mutex, 1);
    }
    return 0;
}

static int lua_audioplaying(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    lua_pushboolean(L, aud->playing);
    return 1;
}

static int lua_audiopaused(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    lua_pushboolean(L, aud->paused);
    return 1;
}

static int lua_audioduration(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");

    switch(aud->type){
        case AUDIO_TYPE_MP3: {
            off_t length = mpg123_length(aud->mp3.handle);
            long rate;
            mpg123_getformat(aud->mp3.handle, &rate, NULL, NULL);
            lua_pushnumber(L, (double)length / rate);
            break;
        }
        case AUDIO_TYPE_WAV: {
            lua_pushnumber(L, (double)aud->wav.wav.totalPCMFrameCount / aud->wav.wav.sampleRate);
            break;
        }
        case AUDIO_TYPE_OGG: {
            lua_pushnumber(L, ov_time_total(&aud->ogg.ogg, -1));
            break;
        }
        case AUDIO_TYPE_FLAC: {
            if (aud->flac.sampleRate > 0) {
                lua_pushnumber(L, (double)aud->flac.totalSamples / aud->flac.sampleRate);
            } else {
                lua_pushnil(L);
            }
            break;
        }
        case AUDIO_TYPE_OPUS: {
            ogg_int64_t total = op_pcm_total(aud->opus.opus, -1);
            const OpusHead *head = op_head(aud->opus.opus, -1);
            lua_pushnumber(L, (double)total / head->input_sample_rate);
            break;
        }
        case AUDIO_TYPE_XM: {
            xmp_get_frame_info(aud->xmp.ctx, &aud->xmp.fi);
            lua_pushnumber(L, (double)aud->xmp.fi.total_time / 1000.0);
            break;
        }
        default:
            lua_pushnil(L);
            break;
    }

    return 1;
}

static int lua_audioelapsed(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    switch(aud->type){
        case AUDIO_TYPE_MP3: {
            off_t elapsed = mpg123_tell(aud->mp3.handle);
            long rate;
            mpg123_getformat(aud->mp3.handle, &rate, NULL, NULL);
            lua_pushnumber(L, (double)elapsed / rate);
            break;
        }
        case AUDIO_TYPE_WAV: {
            lua_pushnumber(L, (double)aud->wav.frames_played / aud->wav.wav.sampleRate);
            break;
        }
        case AUDIO_TYPE_OGG: {
            lua_pushnumber(L, ov_time_tell(&aud->ogg.ogg));
            break;
        }
        case AUDIO_TYPE_FLAC: {
            if (aud->flac.sampleRate > 0) {
                lua_pushnumber(L, (double)aud->flac.playedSamples / aud->flac.sampleRate);
            } else {
                lua_pushnil(L);
            }
            break;
        }
        case AUDIO_TYPE_OPUS: {
            ogg_int64_t elapsed = op_pcm_tell(aud->opus.opus);
            const OpusHead *head = op_head(aud->opus.opus, -1);
            lua_pushnumber(L, (double)elapsed / head->input_sample_rate);
            break;
        }
        case AUDIO_TYPE_RAW: {
            lua_pushnumber(L, (double)aud->raw.frames_played / 48000); // hardcoded samplerate for raw files
            break;
        }
        case AUDIO_TYPE_XM: {
            xmp_get_frame_info(aud->xmp.ctx, &aud->xmp.fi);
            lua_pushnumber(L, (double)aud->xmp.fi.time / 1000.0);
            break;
        }
        default:
            lua_pushnil(L);
            break;
    }
    return 1;
}

static int lua_audiogc(lua_State *L) {
    Audio *aud = (Audio *)luaL_checkudata(L, 1, "audio");
    if (aud->type == AUDIO_TYPE_RAW && aud->raw.file) fclose(aud->raw.file);
    else if (aud->type == AUDIO_TYPE_MP3 && aud->mp3.handle) mpg123_close(aud->mp3.handle), mpg123_delete(aud->mp3.handle);
    else if (aud->type == AUDIO_TYPE_WAV) drwav_uninit(&aud->wav.wav);
    else if (aud->type == AUDIO_TYPE_FLAC && aud->flac.decoder) {
        FLAC__stream_decoder_finish(aud->flac.decoder);
        FLAC__stream_decoder_delete(aud->flac.decoder);
        free(aud->flac.decode_buffer);
    }
    else if (aud->type == AUDIO_TYPE_OGG) ov_clear(&aud->ogg.ogg);
    else if (aud->type == AUDIO_TYPE_OPUS) op_free(aud->opus.opus);
    else if (aud->type == AUDIO_TYPE_XM){
        xmp_release_module(aud->xmp.ctx);
	    xmp_free_context(aud->xmp.ctx);
    }
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

static int lua_comment(lua_State *L){
    Audio *audio = (Audio *)luaL_checkudata(L, 1, "audio");
    if(audio->type == AUDIO_TYPE_OGG){
        vorbis_comment *comment = ov_comment(&audio->ogg.ogg, -1);
        if(comment != NULL){lua_pushnil(L); return 1;}
        lua_newtable(L);
        lua_pushstring(L, vorbis_comment_query(comment, "title", 0)); lua_setfield(L, -2, "title");
        lua_pushstring(L, vorbis_comment_query(comment, "album", 0)); lua_setfield(L, -2, "album");
        lua_pushstring(L, vorbis_comment_query(comment, "artist", 0)); lua_setfield(L, -2, "artist");
        lua_pushstring(L, vorbis_comment_query(comment, "year", 0)); lua_setfield(L, -2, "year");
        lua_pushstring(L, vorbis_comment_query(comment, "comment", 0)); lua_setfield(L, -2, "comment");
        lua_pushstring(L, vorbis_comment_query(comment, "genre", 0)); lua_setfield(L, -2, "genre");
    }else lua_pushnil(L);
    return 1;
}

static int lua_tags(lua_State *L){
    Audio *audio = (Audio *)luaL_checkudata(L, 1, "audio");
    if(audio->type == AUDIO_TYPE_OPUS){
        const OpusTags *tags = op_tags(audio->opus.opus, 0);
        lua_pushstring(L, opus_tags_query(tags, "title", 0)); lua_setfield(L, -2, "title");
        lua_pushstring(L, opus_tags_query(tags, "album", 0)); lua_setfield(L, -2, "album");
        lua_pushstring(L, opus_tags_query(tags, "artist", 0)); lua_setfield(L, -2, "artist");
        lua_pushstring(L, opus_tags_query(tags, "date", 0)); lua_setfield(L, -2, "date");
        lua_pushstring(L, opus_tags_query(tags, "comment", 0)); lua_setfield(L, -2, "comment");
        lua_pushstring(L, opus_tags_query(tags, "genre", 0)); lua_setfield(L, -2, "genre");
        Image *image = (Image *)lua_newuserdata(L, sizeof(Image));
        OpusPictureTag picture_tag = {0};
        opus_picture_tag_init(&picture_tag);
		const char* metadata_block = opus_tags_query(tags, "METADATA_BLOCK_PICTURE", 0);
        int error = opus_picture_tag_parse(&picture_tag, metadata_block);
		if (error == 0) {
			if (picture_tag.type == 3) {
				if (picture_tag.format == OP_PIC_FORMAT_JPEG) image->tex = vita2d_load_JPEG_buffer(picture_tag.data, picture_tag.data_length);
				else if (picture_tag.format == OP_PIC_FORMAT_PNG) image->tex = vita2d_load_PNG_buffer(picture_tag.data);
			}
		}
        luaL_getmetatable(L, "image");
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "image");
		opus_picture_tag_clear(&picture_tag);
    }else lua_pushnil(L);
    return 1;
}

static const luaL_Reg audio_lib[] = {
    {"load", lua_audioload},
    {"play", lua_audioplay},
    {"pause", lua_audiopause},
    {"paused", lua_audiopaused},
    {"playing", lua_audioplaying},
    {"duration", lua_audioduration},
    {"elapsed", lua_audioelapsed},
    {"seek", lua_audioseek},
    {"id3v1", lua_id3v1},
    {"id3v2", lua_id3v2},
    {"comment", lua_comment},
    {"tags", lua_tags},
    {"stop", lua_audiostop},
    {NULL, NULL}
};

static const luaL_Reg audio_methods[] = {
    {"play", lua_audioplay},
    {"pause", lua_audiopause},
    {"paused", lua_audiopaused},
    {"playing", lua_audioplaying},
    {"duration", lua_audioduration},
    {"elapsed", lua_audioelapsed},
    {"seek", lua_audioseek},
    {"id3v1", lua_id3v1},
    {"id3v2", lua_id3v2},
    {"comment", lua_comment},
    {"tags", lua_tags},
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