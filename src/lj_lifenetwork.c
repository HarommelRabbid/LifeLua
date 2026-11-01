/*
    LifeLua WIP
    Network library
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
#include <ftpvita.h>

#include "lj_lifeinit.h"

static int lua_ftp(lua_State *L){
	bool enable = lua_toboolean(L, 1);
	if (enable){
		if (vita_port == 0){
			ftpvita_init(vita_ip, &vita_port);
			lua_newtable(L);
			lua_pushstring(L, vita_ip);
			lua_setfield(L, -2, "ip");
			lua_pushinteger(L, vita_port);
			lua_setfield(L, -2, "port");
		}else{
			return luaL_error(L, "FTP was already started once");
		}
	}else{
		if (vita_port != 0) {
			ftpvita_fini();
			vita_port = 0;
			return 0;
		}else{
			return luaL_error(L, "FTP was already finished once");
		}
	}
	return 1;
}

static int lua_ftpinit(lua_State *L){
	lua_pushboolean(L, ftpvita_is_initialized());
	return 1;
}

static int lua_ftp_add(lua_State *L){
	if (vita_port != 0){
		const char* device = luaL_checkstring(L, 1);
		ftpvita_add_device(device);
	}else{
		return luaL_error(L, "FTP isn't running");
	}
	return 1;
}

static int lua_ftp_del(lua_State *L){
	if (vita_port != 0){
		const char* device = luaL_checkstring(L, 1);
		ftpvita_del_device(device);
	}else{
		return luaL_error(L, "FTP isn't running");
	}
	return 1;
}

static int lua_wifi(lua_State *L){
	int state;
	sceNetCtlInetGetState(&state);
	lua_pushboolean(L, state);
	return 1;
}

static int lua_ip(lua_State *L){
	SceNetCtlInfo info;
	if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0) lua_pushstring(L, "127.0.0.1");
	else lua_pushstring(L, info.ip_address);
	return 1;
}

static int lua_mac(lua_State *L) {
	SceNetEtherAddr mac;
	sceNetGetMacAddress(&mac, 0);
	lua_pushfstring(L, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
	return 1;
}

#define LIFELUA_USERAGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36"
#define HTTP_CHUNK_SIZE (16*1024)

typedef struct {
	int tpl, conn, req;
} HttpHandles;

static void http_close(HttpHandles *h){
	if (h->req >= 0) sceHttpDeleteRequest(h->req);
	if (h->conn >= 0) sceHttpDeleteConnection(h->conn);
	if (h->tpl >= 0) sceHttpDeleteTemplate(h->tpl);
	h->req = h->conn = h->tpl = -1;
}

// Creates the template/connection/request and sends it. On failure everything already created is freed.
static int http_open(HttpHandles *h, const char *url, int method, const char *data, size_t datalen, const char *contenttype, const char *useragent){
	h->tpl = h->conn = h->req = -1;
	h->tpl = sceHttpCreateTemplate(useragent, SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if (h->tpl < 0) goto fail;
	h->conn = sceHttpCreateConnectionWithURL(h->tpl, url, SCE_TRUE);
	if (h->conn < 0) goto fail;
	h->req = sceHttpCreateRequestWithURL(h->conn, method, url, data ? datalen : 0);
	if (h->req < 0) goto fail;
	if (data && contenttype) sceHttpAddRequestHeader(h->req, "Content-Type", contenttype, SCE_HTTP_HEADER_OVERWRITE);
	if (sceHttpSendRequest(h->req, data, data ? datalen : 0) < 0) goto fail;
	return 0;
fail:
	http_close(h);
	return -1;
}

// Calls the global LifeLuaNetworkDownload, if there is one. Returns 0, or non-zero with the error message on the stack.
static int http_progress(lua_State *L, uint64_t downloaded, uint64_t written, uint64_t size, int read, int wrote){
	lua_getglobal(L, "LifeLuaNetworkDownload");
	if (!lua_isfunction(L, -1)){
		lua_pop(L, 1);
		return 0;
	}
	lua_pushnumber(L, downloaded);
	lua_pushnumber(L, written);
	lua_pushnumber(L, size);
	lua_pushnumber(L, read);
	lua_pushnumber(L, wrote);
	return lua_pcall(L, 5, 0, 0);
}

// network.download(url, path, [method], [data], [contenttype], [useragent]) -> status code, or nil on failure
static int lua_download(lua_State *L){
	const char *url = luaL_checkstring(L, 1);
	const char *path = luaL_checkstring(L, 2);
	int method = luaL_optinteger(L, 3, SCE_HTTP_METHOD_GET);
	size_t datalen = 0;
	const char *data = luaL_optlstring(L, 4, NULL, &datalen);
	const char *contenttype = luaL_optstring(L, 5, "application/x-www-form-urlencoded");
	const char *useragent = luaL_optstring(L, 6, LIFELUA_USERAGENT);

	HttpHandles h;
	if (http_open(&h, url, method, data, datalen, contenttype, useragent) < 0){
		lua_pushnil(L);
		return 1;
	}

	int status = 0;
	sceHttpGetStatusCode(h.req, &status);
	unsigned long long size = 0;
	sceHttpGetResponseContentLength(h.req, &size);

	SceUID fh = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fh < 0){
		http_close(&h);
		lua_pushnil(L);
		return 1;
	}
	unsigned char *chunk = malloc(HTTP_CHUNK_SIZE);
	if (!chunk){
		sceIoClose(fh);
		http_close(&h);
		return luaL_error(L, "out of memory");
	}

	uint64_t downloaded = 0, written = 0;
	int read, failed = 0;
	while ((read = sceHttpReadData(h.req, chunk, HTTP_CHUNK_SIZE)) > 0){
		int wrote = sceIoWrite(fh, chunk, read);
		if (wrote < 0){ failed = 1; break; }
		downloaded += read;
		written += wrote;
		if (http_progress(L, downloaded, written, size, read, wrote) != 0){
			// the callback raised an error: clean up, then pass the error on
			free(chunk);
			sceIoClose(fh);
			http_close(&h);
			return lua_error(L);
		}
	}

	free(chunk);
	sceIoClose(fh);
	http_close(&h);
	if (read < 0 || failed){
		lua_pushnil(L);
		return 1;
	}
	lua_pushinteger(L, status);
	return 1;
}

// network.get(url, [method], [data], [contenttype], [useragent]) -> body, status code; or nil on failure
static int lua_get(lua_State *L){
	const char *url = luaL_checkstring(L, 1);
	int method = luaL_optinteger(L, 2, SCE_HTTP_METHOD_GET);
	size_t datalen = 0;
	const char *data = luaL_optlstring(L, 3, NULL, &datalen);
	const char *contenttype = luaL_optstring(L, 4, "application/x-www-form-urlencoded");
	const char *useragent = luaL_optstring(L, 5, LIFELUA_USERAGENT);

	HttpHandles h;
	if (http_open(&h, url, method, data, datalen, contenttype, useragent) < 0){
		lua_pushnil(L);
		return 1;
	}

	int status = 0;
	sceHttpGetStatusCode(h.req, &status);
	unsigned long long size = 0;
	sceHttpGetResponseContentLength(h.req, &size);

	unsigned char *chunk = malloc(HTTP_CHUNK_SIZE);
	if (!chunk){
		http_close(&h);
		return luaL_error(L, "out of memory");
	}

	luaL_Buffer b;
	luaL_buffinit(L, &b);
	uint64_t downloaded = 0;
	int read;
	while ((read = sceHttpReadData(h.req, chunk, HTTP_CHUNK_SIZE)) > 0){
		luaL_addlstring(&b, (const char *)chunk, read);
		downloaded += read;
		// nothing is written to a file here, so "written" is the same as "downloaded"
		if (http_progress(L, downloaded, downloaded, size, read, read) != 0){
			free(chunk);
			http_close(&h);
			return lua_error(L);
		}
	}

	free(chunk);
	http_close(&h);
	if (read < 0){
		lua_pushnil(L);
		return 1;
	}
	luaL_pushresult(&b);
	lua_pushinteger(L, status);
	return 2;
}

// network.header(url, [useragent]) -> all response headers as one string, or nil on failure
static int lua_header(lua_State *L){
	const char *url = luaL_checkstring(L, 1);
	const char *useragent = luaL_optstring(L, 2, LIFELUA_USERAGENT);

	HttpHandles h;
	if (http_open(&h, url, SCE_HTTP_METHOD_GET, NULL, 0, NULL, useragent) < 0){
		lua_pushnil(L);
		return 1;
	}

	char *header;
	unsigned int headerSize;
	if (sceHttpGetAllResponseHeaders(h.req, &header, &headerSize) < 0) lua_pushnil(L);
	else lua_pushlstring(L, header, headerSize); // copied before the request (which owns it) is deleted
	http_close(&h);
	return 1;
}

static const luaL_Reg network_lib[] = {
	{"wifi", lua_wifi},
	{"ip", lua_ip},
	{"mac", lua_mac},
	{"download", lua_download},
	{"get", lua_get},
	{"header", lua_header},
    {NULL, NULL}
};

static const luaL_Reg ftp_lib[] = {
    {"start", lua_ftp},
    {"isrunning", lua_ftpinit},
    {"adddevice", lua_ftp_add},
    {"removedevice", lua_ftp_del},
    {NULL, NULL}
};

LUALIB_API int luaL_opennetwork(lua_State *L) {
	luaL_register(L, "network", network_lib);
	
	lua_newtable(L);
	luaL_register(L, NULL, ftp_lib);
	lua_setfield(L, -2, "ftp"); // network.ftp = ftp_lib

	luaL_pushglobalint(L, SCE_HTTP_METHOD_GET);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_POST);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_HEAD);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_OPTIONS);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_PUT);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_DELETE);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_TRACE);
	luaL_pushglobalint(L, SCE_HTTP_METHOD_CONNECT);
    return 1;
}