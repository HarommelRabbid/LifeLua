/*
    LifeLua WIP
    IO library extension
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
#define str(str) #str
#define luaL_pushglobalint(L, value) do { lua_pushinteger(L, value); lua_setglobal (L, str(value)); } while(0)
#define luaL_pushglobalint_as(L, value, var) do { lua_pushinteger(L, value); lua_setglobal (L, var); } while(0)
#define luaL_pushglobalint_alsoas(L, value, var) do { luaL_pushglobalint(L, value); luaL_pushglobalint_as(L, value, var); } while(0)
#define lerp(value, from_max, to_max) ((((value*10) * (to_max*10))/(from_max*10))/10)

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
                    return luaL_error(L, "Expected a string value for parameter '%s'", param);
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
                    return luaL_error(L, "Expected an integer value for parameter '%s'", param);
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

int recursive_delete(const char *path) {
    SceIoStat stat;
    if (sceIoGetstat(path, &stat) < 0) return -1;

    if (SCE_S_ISDIR(stat.st_mode)) {
        // Open directory
        SceUID dfd = sceIoDopen(path);
        if (dfd < 0) return -1;

        SceIoDirent dir;
        memset(&dir, 0, sizeof(SceIoDirent));

        // Read each entry
        while (sceIoDread(dfd, &dir) > 0) {
            // Skip "." and ".."
            if (strcmp(dir.d_name, ".") == 0 || strcmp(dir.d_name, "..") == 0) continue;

            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dir.d_name);

            // Recurse
            if (recursive_delete(fullpath) < 0) {
                sceIoDclose(dfd);
                return -1;
            }
        }

        sceIoDclose(dfd);
        return sceIoRmdir(path);  // remove the now-empty directory

    } else {
        return sceIoRemove(path);  // remove file
    }
}

static int lua_delete(lua_State *L){
    const char *path = luaL_checkstring(L, 1);
	SceIoStat stat;
	if(sceIoGetstat(path, &stat) < 0){
		lua_pushboolean(L, false);
		return 1;
	}

    if(SCE_S_ISDIR(stat.st_mode)){
		if(sceIoRmdir(path) >= 0) lua_pushboolean(L, true);
		else lua_pushboolean(L, false);
	}else{
		if(sceIoRemove(path) < 0) lua_pushboolean(L, false);
		else lua_pushboolean(L, true);
	}

	return 1;
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

static int lua_sha1(lua_State *L){
	const char *path = luaL_checkstring(L, 1);
	uint8_t sha1out[20];
	// Set up SHA1 context
  	SHA1_CTX ctx;
 	sha1_init(&ctx);

  	SceUID fd = sceIoOpen(path, SCE_O_RDONLY, 0);
  	if (fd < 0) lua_pushnil(L);

  	// Open up the buffer for copying data into
  	void *buf = memalign(4096, (128 * 1024));

  	// Actually take the SHA1 sum
  	while (1) {
    	int read = sceIoRead(fd, buf, (128 * 1024));

    	if (read < 0) {
      		free(buf);
      		sceIoClose(fd);
      		lua_pushnil(L);
			return 1;
    	}

    	if (read == 0)
      		break;

    	sha1_update(&ctx, buf, read);
	}

	sha1_final(&ctx, sha1out);

  	// Free up file buffer
  	free(buf);

  	// Close file properly
  	sceIoClose(fd);

	char sha1msg[42];
  	memset(sha1msg, 0, sizeof(sha1msg));

  	// Construct SHA1 sum string
  	int i;
  	for (i = 0; i < 20; i++) {
    	char string[4];
    	sprintf(string, "%02X", sha1out[i]);
    	strcat(sha1msg, string);
  	}

  	//sha1msg[41] = '\0';

	lua_pushstring(L, sha1msg);

	return 1;
}

static int lua_crc32(lua_State *L) {
	uLong crc = crc32(0L, Z_NULL, 0);
    const char *str = luaL_checkstring(L, 1);
	uInt len = strlen(str);
    crc = crc32(crc, (const Bytef *)str, len);
    lua_pushnumber(L, crc);
    return 1;
}

static const struct luaL_Reg io_lib[] = {
	{"readsfo", lua_readsfo},
	{"editsfo", lua_editsfo},
	{"exists", lua_fileexist},
	{"newfolder", lua_newfolder},
	{"delete", lua_delete},
	{"list", lua_list},
	{"pathstrip", lua_getfilename},
	{"filestrip", lua_getfolder},
	{"sha1", lua_sha1},
	{"crc32", lua_crc32},
    {NULL, NULL}
};

void luaL_extendio(lua_State *L) {
	luaL_openlib(L, "io", io_lib, 0);
}