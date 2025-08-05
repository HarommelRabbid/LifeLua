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
#include <libgen.h>

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/sha1.h"
#include "include/sha256.h"
#include "include/md5.h"
#include <archive.h>
#include <archive_entry.h>

#include "lj_lifeinit.h"

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

static int lua_getfilename(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    char *path_copy = strdup(path);
    lua_pushstring(L, basename(path_copy));
    return 1;
}

static int lua_getfolder(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    char *path_copy = strdup(path);
    lua_pushstring(L, dirname(path_copy));
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

static int lua_sha256(lua_State *L){
	const char *input = luaL_checkstring(L, 1);
	size_t len = strlen(input);

	uint8_t sha256out[20];

	// Set up SHA1 context
	SHA256_CTX ctx;
	sha256_init(&ctx);
	sha256_update(&ctx, (const uint8_t *)input, len);
	sha256_final(&ctx, sha256out);

	// Convert SHA256 result to hex string
	char sha256msg[42]; // 40 chars + null terminator
	for (int i = 0; i < 20; i++) {
		sprintf(sha256msg + i * 2, "%02X", sha256out[i]);
	}

	lua_pushstring(L, sha256msg);
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
    const char *archive_path = luaL_checkstring(L, 1);
    const char *out_path = luaL_checkstring(L, 2);

    struct archive *a = archive_read_new();
    struct archive_entry *entry;
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (archive_read_open_filename(a, archive_path, 10240) != ARCHIVE_OK) {
        archive_read_free(a);
        return luaL_error(L, "%s", archive_error_string(a));
    }

    int file_index = 1;

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {

        // Build full output path
        const char *entry_path = archive_entry_pathname(entry);
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s/%s", out_path, entry_path);

        // Ensure directory structure
        char *last_slash = strrchr(full_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            recursive_folders(full_path);
            *last_slash = '/';
        }

        lua_getglobal(L, "LifeLuaArchiveExtracting");
        if (lua_isfunction(L, -1)) {
            lua_pushstring(L, archive_entry_pathname(entry));
            lua_pushinteger(L, file_index++);
            //lua_pushnumber(L, archive_entry_);
            lua_pushnumber(L, archive_entry_size(entry));
            lua_call(L, 3, 0);
        } else {
            lua_pop(L, 1); // not a function
        }

        // Skip directories
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            archive_read_data_skip(a);
            continue;
        }

        SceUID fd = sceIoOpen(full_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
        if (fd < 0) {
            archive_read_free(a);
            return luaL_error(L, "Failed to open output file: %s", full_path);
        }

        char buffer[4096];
        ssize_t size;
        while ((size = archive_read_data(a, buffer, sizeof(buffer))) > 0) {
            sceIoWrite(fd, buffer, size);
        }

        sceIoClose(fd);
    }

    archive_read_close(a);
    archive_read_free(a);
    return 0;
}

static int add_file_to_archive(struct archive *a, const char *src_path, const char *entry_name) {
    struct archive_entry *entry = archive_entry_new();
    archive_entry_set_pathname(entry, entry_name);
    archive_entry_set_filetype(entry, AE_IFREG);
    struct stat stt;
    stat(src_path, &stt);

    SceIoStat st;
    if (sceIoGetstat(src_path, &st) >= 0) {
        archive_entry_set_size(entry, st.st_size);
    }

    archive_entry_set_mtime(entry, mktime(localtime(&stt.st_mtime)), 0);
    archive_entry_set_atime(entry, mktime(localtime(&stt.st_atime)), 0);
    archive_entry_set_ctime(entry, mktime(localtime(&stt.st_ctime)), 0);

    archive_write_header(a, entry);

    SceUID fd = sceIoOpen(src_path, SCE_O_RDONLY, 0);
    if (fd >= 0) {
        char buffer[4096];
        int bytes;
        while ((bytes = sceIoRead(fd, buffer, sizeof(buffer))) > 0) {
            archive_write_data(a, buffer, bytes);
        }
        sceIoClose(fd);
    }

    archive_entry_free(entry);
    return 1;
}

static int add_folder_recursive(struct archive *a, const char *real_path, const char *zip_root) {
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
            char foldername[512];
            if (snprintf(foldername, sizeof(foldername), "%s/", relative_zip_path) >= sizeof(foldername)) return 0; // path too long
            struct archive_entry *dir_entry = archive_entry_new();
            archive_entry_set_pathname(dir_entry, foldername);
            archive_entry_set_filetype(dir_entry, AE_IFDIR);
            archive_entry_set_size(dir_entry, 0);
            archive_write_header(a, dir_entry);
            archive_entry_free(dir_entry);

            add_folder_recursive(a, full_path, relative_zip_path);
        } else {
            add_file_to_archive(a, full_path, relative_zip_path);
        }
    }

    sceIoDclose(dir);
    return 1;
}

static int lua_create(lua_State *L) {
    const char *archive_path = luaL_checkstring(L, 2);
    const char *format = luaL_optstring(L, 3, "zip");

    struct archive *a = archive_write_new();
    if(strcasecmp(format, "zip") == 0) archive_write_set_format_zip(a);
    else if(strcasecmp(format, "7z") == 0 || strcasecmp(format, "7zip") == 0){
        if(archive_write_set_format_7zip(a) != ARCHIVE_OK) return luaL_error(L, archive_error_string(a));
    }
    else if(strcasecmp(format, "gnutar") == 0) archive_write_set_format_gnutar(a);
    else if(strcasecmp(format, "xar") == 0) archive_write_set_format_xar(a);
    else if(strcasecmp(format, "iso9660") == 0){
        if(archive_write_set_format_iso9660(a) != ARCHIVE_OK) return luaL_error(L, archive_error_string(a));
    }
    else if(strcasecmp(format, "raw") == 0) archive_write_set_format_raw(a);
    else if(strcasecmp(format, "ar_bsd") == 0) archive_write_set_format_ar_bsd(a);
    else if(strcasecmp(format, "ar_svr4") == 0) archive_write_set_format_ar_svr4(a);
    else if(strcasecmp(format, "v7tar") == 0) archive_write_set_format_v7tar(a);
    else if(strcasecmp(format, "ustar") == 0) archive_write_set_format_ustar(a);
    else if(strcasecmp(format, "warc") == 0) archive_write_set_format_warc(a);
    archive_write_open_filename(a, archive_path);

    if (lua_type(L, 1) == LUA_TTABLE) {
        lua_pushnil(L);
        while (lua_next(L, 1) != 0) {
            const char *src_path = luaL_checkstring(L, -2);
            const char *zip_name = luaL_checkstring(L, -1);
            add_file_to_archive(a, src_path, zip_name);
            lua_pop(L, 1);
        }
    } else if (lua_type(L, 2) == LUA_TSTRING) {
        const char *folder = luaL_checkstring(L, 1);
        add_folder_recursive(a, folder, "");
    } else {
        archive_write_close(a);
        archive_write_free(a);
        return luaL_typerror(L, 1, "table or string");
    }

    archive_write_close(a);
    archive_write_free(a);
    lua_pushboolean(L, 1);
    return 1;
}

static int lua_info(lua_State *L){
    const char *path = luaL_checkstring(L, 1);
    SceIoStat stat;
	int res = sceIoGetstat(path, &stat);
	if(res >= 0){
        lua_newtable(L);
        push_datetime(L, &stat.st_atime); lua_setfield(L, -2, "accessed");
        push_datetime(L, &stat.st_mtime); lua_setfield(L, -2, "modified");
        push_datetime(L, &stat.st_ctime); lua_setfield(L, -2, "created");
        lua_pushboolean(L, SCE_S_ISDIR(stat)); lua_setfield(L, -2, "isafolder");
        lua_pushnumber(L, stat.st_size); lua_setfield(L, -2, "size");
    }
	else lua_pushnil(L);
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
    {"sha256", lua_sha256},
	{"crc32", lua_crc32},
    {"md5", lua_md5},
    {"workpath", lua_workpath},
    {"freespace", lua_freespace},
    {"fullspace", lua_totalspace},
    {"extract", lua_extract},
    {"archive", lua_create},
    {"info", lua_info},
    {NULL, NULL}
};

LUALIB_API int luaL_extendio(lua_State *L) {
	luaL_register(L, "io", io_lib);
    return 1;
}