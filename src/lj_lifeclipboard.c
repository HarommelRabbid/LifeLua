/*
    LifeLua PS Vita
    Clipboard library
*/

#include "lj_lifeinit.h"

#include <psp2/sysmodule.h>

#define SCE_CLIPBOARD_MAX_STRING_LENGTH		8189

#define SCE_CLIPBOARD_ERROR_INTERNAL				-2146417152	/* 0x80104600 */
#define SCE_CLIPBOARD_ERROR_NULL_POINTER			-2146417151	/* 0x80104601 */
#define SCE_CLIPBOARD_ERROR_TEXT_TOO_LONG			-2146417150	/* 0x80104602 */
#define SCE_CLIPBOARD_ERROR_UNSUPPORTED_CHARACTER	-2146417149	/* 0x80104603 */

SceInt32 sceClipboardSetText(const SceWChar16* newContents, SceUInt length);

SceInt32 sceClipboardGetText(SceWChar16* buffer, SceUInt maxLength, SceUInt* pLength);

static int clipboard_get(lua_State *L){
    SceUInt len;
    SceWChar16 buf[SCE_CLIPBOARD_MAX_STRING_LENGTH+1];
    sceClipboardGetText(buf, SCE_CLIPBOARD_MAX_STRING_LENGTH, &len);
    char res[SCE_CLIPBOARD_MAX_STRING_LENGTH+1];
    utf2ascii(res, buf);
    lua_pushstring(L, res);
    return 1;
}

static int clipboard_set(lua_State *L){
    SceWChar16 buf[SCE_CLIPBOARD_MAX_STRING_LENGTH+1];
    const char *str = luaL_checkstring(L, 1);
    ascii2utf(buf, str);
    sceClipboardSetText(buf, SCE_CLIPBOARD_MAX_STRING_LENGTH);
    return 0;
}

static const luaL_Reg clipboard_lib[] = {
    {"set", clipboard_set},
    {"get", clipboard_get},
    {NULL, NULL}
};

LUALIB_API int luaL_openclipboard(lua_State *L){
    luaL_register(L, "clipboard", clipboard_lib);
    return 1;
}