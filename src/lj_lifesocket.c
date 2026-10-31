/*
    LifeLua PS Vita
    Socket library
*/

#include "lj_lifeinit.h"

typedef struct {
    int fd;
    SceNetSocketType type;
} Socket;

static int socket_connect(lua_State *L){
    const char *ip = luaL_checkstring(L, 1);
    unsigned int port = luaL_checkinteger(L, 2);
    SceNetProtocol proto = luaL_optinteger(L, 3, SCE_NET_IPPROTO_TCP);
    SceNetSocketType type = (proto == SCE_NET_IPPROTO_UDP) ? SCE_NET_SOCK_DGRAM : SCE_NET_SOCK_STREAM;
    int fd = sceNetSocket("LifeLuaClient", SCE_NET_AF_INET, type, proto);

    SceNetSockaddrIn addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = SCE_NET_AF_INET;
    addr.sin_port = sceNetHtons(port);
    sceNetInetPton(SCE_NET_AF_INET, ip, &addr.sin_addr);

    if (sceNetConnect(fd, (SceNetSockaddr*)&addr, sizeof(addr)) < 0) {
        sceNetSocketClose(fd);
        return luaL_error(L, "Connection failed");
    }

    Socket *sock = (Socket *)lua_newuserdata(L, sizeof(Socket));
    sock->fd = fd;
    sock->type = type;

    luaL_getmetatable(L, "socket");
    lua_setmetatable(L, -2);
    return 1;
}

static int socket_host(lua_State *L){
    unsigned int port = luaL_checkinteger(L, 1);
    SceNetProtocol proto = luaL_optinteger(L, 2, SCE_NET_IPPROTO_TCP);
    SceNetSocketType type = (proto == SCE_NET_IPPROTO_UDP) ? SCE_NET_SOCK_DGRAM : SCE_NET_SOCK_STREAM;
    int fd = sceNetSocket("LifeLuaHost", SCE_NET_AF_INET, type, proto);

    SceNetSockaddrIn addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = SCE_NET_AF_INET;
    addr.sin_port = sceNetHtons(port);
    addr.sin_addr.s_addr = sceNetHtonl(SCE_NET_INADDR_ANY);

    if (sceNetBind(fd, (SceNetSockaddr*)&addr, sizeof(addr)) < 0) {
        sceNetSocketClose(fd);
        return luaL_error(L, "Failed to bind port");
    }

    if (type == SCE_NET_SOCK_STREAM) sceNetListen(fd, 16);

    Socket *sock = (Socket *)lua_newuserdata(L, sizeof(Socket));
    sock->fd = fd;
    sock->type = type;
    
    luaL_getmetatable(L, "socket");
    lua_setmetatable(L, -2);
    return 1;
}

static int socket_bind(lua_State *L) {
    Socket *host = (Socket*)luaL_checkudata(L, 1, "socket");
    if (host->type != SCE_NET_SOCK_STREAM) return luaL_error(L, "Cannot bind on UDP socket");

    SceNetSockaddrIn client_addr;
    unsigned int addr_len = sizeof(client_addr);
    
    int client_fd = sceNetAccept(host->fd, (SceNetSockaddr*)&client_addr, &addr_len);
    
    if (client_fd < 0) {
        if (client_fd == SCE_NET_ERROR_EAGAIN) return lua_pushnil(L), 1;
        return luaL_error(L, "Bind failed (%x)", client_fd);
    }

    Socket *sock = (Socket *)lua_newuserdata(L, sizeof(Socket));
    sock->fd = client_fd;
    sock->type = SCE_NET_SOCK_STREAM;
    
    luaL_getmetatable(L, "socket");
    lua_setmetatable(L, -2);
    return 1;
}

static int socket_send(lua_State *L) {
    Socket *sock = (Socket*)luaL_checkudata(L, 1, "socket");
    size_t len;
    const char *data = luaL_checklstring(L, 2, &len);
    
    size_t send_len = luaL_optinteger(L, 3, len);

    int sent = sceNetSend(sock->fd, data, send_len, 0);
    lua_pushinteger(L, sent);
    return 1;
}

static int socket_receive(lua_State *L) {
    Socket *sock = (Socket*)luaL_checkudata(L, 1, "socket");
    size_t req_len = luaL_optinteger(L, 2, 1024); // Default to 1KB chunks
    
    char buffer[req_len];
    int bytes_read = sceNetRecv(sock->fd, buffer, req_len, 0);
    
    if (bytes_read < 0) {
        if (bytes_read == SCE_NET_ERROR_EAGAIN) return lua_pushnil(L), 1;
        return luaL_error(L, "Socket receive error (%x)", bytes_read);
    }

    if (bytes_read == 0) return lua_pushnil(L), 1;

    lua_pushlstring(L, buffer, bytes_read);
    return 1;
}

static int socket_receivefrom(lua_State *L) {
    Socket *sock = (Socket*)luaL_checkudata(L, 1, "socket");
    size_t req_len = luaL_optinteger(L, 2, 1024);
    
    char buffer[req_len];
    SceNetSockaddrIn sender_addr;
    unsigned int addr_len = sizeof(sender_addr);
    
    int bytes_read = sceNetRecvfrom(sock->fd, buffer, req_len, 0, (SceNetSockaddr*)&sender_addr, &addr_len);
    
    if (bytes_read < 0) {
        if (bytes_read == SCE_NET_ERROR_EAGAIN) return lua_pushnil(L), 1;
        return luaL_error(L, "Socket receivefrom error: %x", bytes_read);
    }
    
    if (bytes_read == 0) return lua_pushnil(L), 1;

    char ip_str[16];
    sceNetInetNtop(SCE_NET_AF_INET, &sender_addr.sin_addr, ip_str, sizeof(ip_str));

    int port = sceNetNtohs(sender_addr.sin_port);

    lua_pushlstring(L, buffer, bytes_read);
    lua_pushstring(L, ip_str);
    lua_pushinteger(L, port);
    
    return 3;
}

static int socket_sendto(lua_State *L) {
    Socket *sock = (Socket*)luaL_checkudata(L, 1, "socket");
    
    size_t len;
    const char *data = luaL_checklstring(L, 2, &len);
    const char *ip = luaL_checkstring(L, 3);
    int port = luaL_checkinteger(L, 4);
    
    SceNetSockaddrIn dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = SCE_NET_AF_INET;
    dest_addr.sin_port = sceNetHtons(port);
    sceNetInetPton(SCE_NET_AF_INET, ip, &dest_addr.sin_addr);

    int sent = sceNetSendto(sock->fd, data, len, 0, (SceNetSockaddr*)&dest_addr, sizeof(dest_addr));
                           
    if (sent < 0) return luaL_error(L, "Socket sendto error: %x", sent);
    
    lua_pushinteger(L, sent);
    return 1;
}

static int socket_close(lua_State *L) {
    Socket *sock = (Socket*)luaL_checkudata(L, 1, "socket");
    if (sock->fd >= 0) {
        sceNetSocketClose(sock->fd);
        sock->fd = -1;
    }
    return 0;
}

static int socket_setnonblocking(lua_State *L){
    Socket *sock = (Socket *)luaL_checkudata(L, 1, "socket");
    int nonblocking = lua_toboolean(L, 2);
    int res = sceNetSetsockopt(sock->fd, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nonblocking, sizeof(nonblocking));
    if (res < 0) return luaL_error(L, "Failed to set non-blocking mode (%x)", res);
    return 0;
}

static int socket_setbroadcast(lua_State *L){
    Socket *sock = (Socket *)luaL_checkudata(L, 1, "socket");
    int broadcast = lua_toboolean(L, 2);
    int res = sceNetSetsockopt(sock->fd, SCE_NET_SOL_SOCKET, SCE_NET_SO_BROADCAST, &broadcast, sizeof(broadcast));
    if (res < 0) return luaL_error(L, "Failed to set broadcast mode (%x)", res);
    return 0;
}

static const luaL_Reg socket_lib[] = {
    {"connect", socket_connect},
    {"host", socket_host},
    {"bind", socket_bind},
    {"send", socket_send},
    {"recieve", socket_receive},
    {"sendto", socket_sendto},
    {"recievefrom", socket_receivefrom},
    {"setnonblocking", socket_setnonblocking},
    {"setbroadcast", socket_setbroadcast},
    {"close", socket_close},
    {NULL, NULL}
};

static const luaL_Reg socket_methods[] = {
    {"bind", socket_bind},
    {"send", socket_send},
    {"recieve", socket_receive},
    {"sendto", socket_sendto},
    {"recievefrom", socket_receivefrom},
    {"setnonblocking", socket_setnonblocking},
    {"setbroadcast", socket_setbroadcast},
    {"close", socket_close},
    {NULL, NULL}
};

LUALIB_API int luaL_opensocket(lua_State *L){
    luaL_newmetatable(L, "socket");

    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2);  /* pushes the metatable */
    lua_settable(L, -3);  /* metatable.__index = metatable */
    
    luaL_register(L, NULL, socket_methods);
    luaL_register(L, "socket", socket_lib);
    luaL_pushglobalint(L, SCE_NET_IPPROTO_TCP);
    luaL_pushglobalint(L, SCE_NET_IPPROTO_UDP);
    return 1;
}