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
#include "include/md5.h"
#include "include/unzip.h"
#include "include/zip.h"

#include "lj_lifeinit.h"
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

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
        sprintf(fullpath, "%s/%s", path, dirent.d_name);
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
        if(out_dir[strlen(out_dir)] == '/') out_dir[strlen(out_dir)] = '\0';
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
	const char *input = luaL_checkstring(L, 1);
	size_t len = strlen(input);

	uint8_t sha1out[20];

	// Set up SHA1 context
	SHA1_CTX ctx;
	sha1_init(&ctx);
	sha1_update(&ctx, (const uint8_t *)input, len);
	sha1_final(&ctx, sha1out);

	// Convert SHA1 result to hex string
	char sha1msg[42]; // 40 chars + null terminator
	for (int i = 0; i < 20; i++) {
		sprintf(sha1msg + i * 2, "%02X", sha1out[i]);
	}

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

static int lua_md5(lua_State *L){
    size_t len;
    const char *input = luaL_checklstring(L, 1, &len);

    unsigned char digest[16];
    struct MD5Context ctx;

    MD5Init(&ctx);
    MD5Update(&ctx, (const uint8_t *)input, (unsigned)len);
    MD5Final(digest, &ctx);

    char hex[33];
    for (int i = 0; i < 16; ++i)
        sprintf(hex + i * 2, "%02X", digest[i]);

    lua_pushstring(L, hex);
    return 1;
}

static int lua_workpath(lua_State *L) {
    if(!lua_isnone(L, 1)){
        const char *path = luaL_checkstring(L, 1);

        // Try changing the working directory
        if (chdir(path) != 0) {
            return luaL_error(L, "Failed to change directory to: %s", path);
        }

        return 0; // no return value
    }else{
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            return luaL_error(L, "Failed to get current working directory");
        }
        lua_pushstring(L, cwd);
        return 1;
    }
}

static int lua_freespace(lua_State *L) {
	uint64_t free_storage = 0;
	uint64_t dummy;
	const char *dev_name = luaL_checkstring(L, 1);
	SceIoDevInfo info;
	sceClibMemset(&info, 0, sizeof(SceIoDevInfo));
	int res = sceIoDevctl(dev_name, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res >= 0)
		free_storage = info.free_size;
	else
		sceAppMgrGetDevInfo(dev_name, &dummy, &free_storage);
	lua_pushnumber(L, free_storage);
	return 1;
}

static int lua_totalspace(lua_State *L) {
	uint64_t total_storage = 0;
	uint64_t dummy;
	const char *dev_name = luaL_checkstring(L, 1);
	SceIoDevInfo info;
	sceClibMemset(&info, 0, sizeof(SceIoDevInfo));
	int res = sceIoDevctl(dev_name, 0x3001, NULL, 0, &info, sizeof(SceIoDevInfo));
	if (res >= 0)
		total_storage = info.max_size;
	else
		sceAppMgrGetDevInfo(dev_name, &total_storage, &dummy);
	lua_pushnumber(L, total_storage);
	return 1;
}

int recursive_folders(const char* path) {
	if(file_exists(path)) return false;
	
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
	
	return sceIoMkdir(path, 0777);
}

static int lua_extract(lua_State *L) {
    const char *zip_path = luaL_checkstring(L, 1);
    const char *out_path = luaL_checkstring(L, 2);

    unzFile zip = unzOpen(zip_path);
    if (!zip)
        return luaL_error(L, "Failed to open ZIP file");

    if (unzGoToFirstFile(zip) != UNZ_OK) {
        unzClose(zip);
        return luaL_error(L, "ZIP file is empty or corrupted");
    }

    unz_global_info global_info;
    unzGetGlobalInfo(zip, &global_info);

    int f_i = 1;

    do {
        char filename_inzip[256];
        unz_file_info file_info;
        if (unzGetCurrentFileInfo(zip, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0) != UNZ_OK)
            continue;

        lua_getglobal(L, "LifeLuaArchiveExtracting");
        if (lua_isfunction(L, -1)) {
            lua_pushstring(L, filename_inzip);
            lua_pushinteger(L, f_i++); //curr file
            lua_pushnumber(L, global_info.number_entry); //total number of archive entries
            lua_pushnumber(L, file_info.compressed_size); //curr size
            lua_pushnumber(L, file_info.uncompressed_size); //total size
            lua_pushnumber(L, file_info.crc); //crc32
            lua_call(L, 6, 0);
        }

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", out_path, filename_inzip);

        if (filename_inzip[strlen(filename_inzip) - 1] == '/') {
            recursive_folders(full_path);
            continue;
        } else {
            char folder_only[512];
            strncpy(folder_only, full_path, sizeof(folder_only));
            char *last_slash = strrchr(folder_only, '/');
            if (last_slash) {
                *last_slash = '\0';
                recursive_folders(folder_only); // make sure parent dirs exist
            }

            // Then open file and extract it
            SceUID fd = sceIoOpen(full_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
            if (fd < 0) {
                unzCloseCurrentFile(zip);
                unzClose(zip);
                return luaL_error(L, "Failed to open output file: %s", full_path);
            }
        }

        if (unzOpenCurrentFile(zip) != UNZ_OK)
            continue;

        SceUID fd = sceIoOpen(full_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd < 0) {
            unzCloseCurrentFile(zip);
            unzClose(zip);
            return luaL_error(L, "Failed to open output file: %s", full_path);
        }

        char buffer[4096];
        int bytes;
        while ((bytes = unzReadCurrentFile(zip, buffer, 4096)) > 0) {
            sceIoWrite(fd, buffer, bytes);
        }

        sceIoClose(fd);
        unzCloseCurrentFile(zip);

    } while (unzGoToNextFile(zip) == UNZ_OK);

    unzClose(zip);
    lua_pushboolean(L, true);
    return 1;
}

void convertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc) {
  // sceRtcGetTick and other sceRtc functions fails with year > 9999
  int year_utc = time_utc->year;
  int year_delta = year_utc < 9999 ? 0 : year_utc - 9998;
  time_utc->year -= year_delta;

  SceRtcTick tick;
  sceRtcGetTick(time_utc, &tick);
  time_utc->year = year_utc;

  sceRtcConvertUtcToLocalTime(&tick, &tick);
  sceRtcSetTick(time_local, &tick);  
  time_local->year += year_delta;
}

void convertLocalTimeToUtc(SceDateTime *time_utc, SceDateTime *time_local) {
  // sceRtcGetTick and other sceRtc functions fails with year > 9999
  int year_local = time_local->year;
  int year_delta = year_local < 9999 ? 0 : year_local - 9998;
  time_local->year -= year_delta;

  SceRtcTick tick;
  sceRtcGetTick(time_local, &tick);
  time_local->year = year_local;

  sceRtcConvertLocalTimeToUtc(&tick, &tick);
  sceRtcSetTick(time_utc, &tick);  
  time_utc->year += year_delta;
}

// Add a single file to the zip (src_path on disk, zip_path inside zip)
static int add_file_to_zip(zipFile zip, const char *src_path, const char *zip_path) {
    zip_fileinfo zi = {0};

    // Get file modification time for the ZIP header
    SceIoStat stat;
    if (sceIoGetstat(src_path, &stat) >= 0) {
        SceDateTime time_local;
        SceDateTime *t = &stat.st_mtime;
        convertUtcToLocalTime(&time_local, t);
        zi.tmz_date.tm_year = time_local.year;
        zi.tmz_date.tm_mon  = time_local.month - 1;
        zi.tmz_date.tm_mday = time_local.day;
        zi.tmz_date.tm_hour = time_local.hour;
        zi.tmz_date.tm_min  = time_local.minute;
        zi.tmz_date.tm_sec  = time_local.second;
    }

    if (zipOpenNewFileInZip(zip, zip_path, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) != ZIP_OK)
        return 0;

    SceUID fd = sceIoOpen(src_path, SCE_O_RDONLY, 0);
    if (fd < 0) {
        zipCloseFileInZip(zip);
        return 0;
    }

    char buffer[4096];
    int bytes;
    while ((bytes = sceIoRead(fd, buffer, 4096)) > 0) {
        if (zipWriteInFileInZip(zip, buffer, bytes) != ZIP_OK) {
            sceIoClose(fd);
            zipCloseFileInZip(zip);
            return 0;
        }
    }

    sceIoClose(fd);
    zipCloseFileInZip(zip);
    return 1;
}

// Recursively add files from a folder
static int add_folder_recursive(zipFile zip, const char *real_path, const char *zip_root) {
    SceUID dir = sceIoDopen(real_path);
    if (dir < 0) return 0;

    SceIoDirent entry;
    memset(&entry, 0, sizeof(SceIoDirent));

    while (sceIoDread(dir, &entry) > 0) {
        if (entry.d_name[0] == '.') continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", real_path, entry.d_name);

        char relative_zip_path[512];
        if (zip_root && strlen(zip_root) > 0)
            snprintf(relative_zip_path, sizeof(relative_zip_path), "%s/%s", zip_root, entry.d_name);
        else
            snprintf(relative_zip_path, sizeof(relative_zip_path), "%s", entry.d_name);

        if (SCE_S_ISDIR(entry.d_stat.st_mode)) {
            add_folder_recursive(zip, full_path, relative_zip_path);
        } else {
            if (!add_file_to_zip(zip, full_path, relative_zip_path)) {
                sceIoDclose(dir);
                return 0;
            }
        }
    }

    sceIoDclose(dir);
    return 1;
}

static int lua_create(lua_State *L) {
    const char *zip_path = luaL_checkstring(L, 2);

    zipFile zip = zipOpen(zip_path, APPEND_STATUS_CREATE);
    if (!zip)
        return luaL_error(L, "Failed to create zip: %s", zip_path);

    if (lua_type(L, 1) == LUA_TTABLE) {
        // Table-based manual file listing
        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            const char *src_path = luaL_checkstring(L, -2);
            const char *zip_name = luaL_checkstring(L, -1);

            if (!add_file_to_zip(zip, src_path, zip_name)) {
                zipClose(zip, NULL);
                return luaL_error(L, "Failed to add file: %s", src_path);
            }

            lua_pop(L, 1);
        }
    } else if (lua_type(L, 2) == LUA_TSTRING) {
        // Folder-based automatic ZIP
        const char *folder = luaL_checkstring(L, 1);
        char base_path[256];
        const char *slash = strrchr(folder, '/');
        if (slash)
            strncpy(base_path, slash + 1, sizeof(base_path));
        else
            strncpy(base_path, folder, sizeof(base_path));
        base_path[sizeof(base_path) - 1] = '\0'; // ensure null-terminated

        if (!add_folder_recursive(zip, folder, "")) {
            zipClose(zip, NULL);
            return luaL_error(L, "Failed to zip folder: %s", folder);
        }
    } else {
        zipClose(zip, NULL);
        return luaL_typerror(L, 1, "table or string");
    }

    zipClose(zip, NULL);
    lua_pushboolean(L, true);
    return 1;
}

static const luaL_Reg io_lib[] = {
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
    {"md5", lua_md5},
    {"workpath", lua_workpath},
    {"freespace", lua_freespace},
    {"fullspace", lua_totalspace},
    {"extract", lua_extract},
    {"archive", lua_create},
    {NULL, NULL}
};

LUALIB_API int luaL_extendio(lua_State *L) {
	luaL_register(L, "io", io_lib);
    return 1;
}