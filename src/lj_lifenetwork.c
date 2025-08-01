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
#include "include/ftpvita.h"
//#include "include/sha1.h"

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
	if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0) strcpy(vita_ip, "127.0.0.1");
	else strcpy(vita_ip, info.ip_address);
	lua_pushstring(L, vita_ip);
	return 1;
}

static int lua_mac(lua_State *L) {
	SceNetEtherAddr mac;
	sceNetGetMacAddress(&mac, 0);
	lua_pushfstring(L, "%02X:%02X:%02X:%02X:%02X:%02X", mac.data[0], mac.data[1], mac.data[2], mac.data[3], mac.data[4], mac.data[5]);
	return 1;
}

static int lua_download(lua_State *L){
	const char *url = luaL_checkstring(L, 1);
	const char *path = luaL_checkstring(L, 2);
	const char *template = luaL_optstring(L, 3, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
	int tpl = sceHttpCreateTemplate(template, SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if(tpl < 0){
		lua_pushnil(L);
		return 1;
	}
	int conn = sceHttpCreateConnectionWithURL(tpl, url, SCE_TRUE);
	if(conn < 0){
		lua_pushnil(L);
		return 1;
	}

	int req = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);
	if(req < 0){
		lua_pushnil(L);
		return 1;
	}

	int res = sceHttpSendRequest(req, NULL, 0);
	if(res < 0){
		lua_pushnil(L);
		return 1;
	}

	int fh = sceIoOpen(path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	unsigned char data[16*1024];
	int read;
	int luaread = 0, luawrote = 0;
	uint64_t size = 0;
	sceHttpGetResponseContentLength(req, &size);

	// read data until finished
	while ((read = sceHttpReadData(req, &data, sizeof(data))) > 0) {
		//psvDebugScreenPrintf("read %d bytes\n", read);

		// writing the count of read bytes from the data buffer to the file
		int written = sceIoWrite(fh, data, read);
		//psvDebugScreenPrintf("wrote %d bytes\n", write);
		lua_getglobal(L, "LifeLuaNetworkDownload");
		if (lua_isfunction(L, -1)) {
			lua_pushnumber(L, luaread+=read);
			lua_pushnumber(L, luawrote+=written);
			lua_pushnumber(L, size);
			lua_pushnumber(L, read);
			lua_pushnumber(L, written);
			lua_call(L, 5, 0);
		}
	}
	sceIoClose(fh);
    if (req >= 0) sceHttpDeleteRequest(req);
    if (conn >= 0) sceHttpDeleteConnection(conn);
    if (tpl >= 0) sceHttpDeleteTemplate(tpl);
	return 1;
}

static int lua_header(lua_State *L){
    char *header;
    unsigned int headerSize;
	const char *url = luaL_checkstring(L, 1);
	const char *template = luaL_optstring(L, 2, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36");
	int tpl = sceHttpCreateTemplate(template, SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if(tpl < 0){
		lua_pushnil(L);
		return 1;
	}
	int conn = sceHttpCreateConnectionWithURL(tpl, url, SCE_TRUE);
	if(conn < 0){
		lua_pushnil(L);
		return 1;
	}

	int req = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);
	if(req < 0){
		lua_pushnil(L);
		return 1;
	}

	int res = sceHttpSendRequest(req, NULL, 0);
	if(res < 0){
		lua_pushnil(L);
		return 1;
	}

	res = sceHttpGetAllResponseHeaders(req, &header, &headerSize);
    if(res < 0){
		lua_pushnil(L);
		return 1;
	}
    lua_pushlstring(L, header, headerSize);
    if (req >= 0) sceHttpDeleteRequest(req);
    if (conn >= 0) sceHttpDeleteConnection(conn);
    if (tpl >= 0) sceHttpDeleteTemplate(tpl);
	return 1;
}

static const luaL_Reg network_lib[] = {
	{"ftp", lua_ftp},
    {"ftpinit", lua_ftpinit},
	{"ftpadddevice", lua_ftp_add},
	{"ftpremovedevice", lua_ftp_del},
	{"wifi", lua_wifi},
	{"ip", lua_ip},
	{"mac", lua_mac},
	{"download", lua_download},
    {"header", lua_header},
    {NULL, NULL}
};

LUALIB_API int luaL_opennetwork(lua_State *L) {
	luaL_register(L, "network", network_lib);
    return 1;
}