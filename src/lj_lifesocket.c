/*
    LifeLua by Harommel
    Socket library
*/

#include "lj_lifeinit.h"

typedef struct {
    int fd;
    int type;
} Socket;

static int socket_connect(lua_State *L){
    const char *ip = luaL_checkstring(L, 1);
    unsigned int port = luaL_checkinteger(L, 2);
    SceNetProtocol proto = luaL_optinteger(L, 3, SCE_NET_IPPROTO_TCP);
    SceNetSocketType type = (proto == SCE_NET_IPPROTO_UDP) ? SCE_NET_SOCK_DGRAM : SCE_NET_SOCK_STREAM;
    int fd = sceNetSocket("LifeLuaSocket", SCE_NET_AF_INET, type, proto);
    return 1;
}

static const luaL_Reg socket_lib[] = {
    {"connect", socket_connect},
    {NULL, NULL}
};

LUALIB_API int luaL_opensocket(lua_State *L){
    luaL_pushglobalint(L, SCE_NET_IPPROTO_TCP);
    luaL_pushglobalint(L, SCE_NET_IPPROTO_UDP);
    return 1;
}