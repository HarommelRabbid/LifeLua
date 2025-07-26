/*
    LifeLua WIP
    JSON library
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
#include <string.h>

#include <vitasdk.h>
#include <taihen.h>
#include <vita2d.h>
#include "include/cJSON.h"

#include "lj_lifeinit.h"

static int lua_jsonnulltostring(lua_State *L) {
	lua_pushstring(L, "json.null");
	return 1;
}

static const luaL_Reg json_null[] = {
    {"__tostring", lua_jsonnulltostring},
    //{"__eq", lua_jsonnulleq},
    {NULL, NULL}
};

static void lua_pushjsonnull(lua_State *L) {
	void **ud = (void **)lua_newuserdata(L, sizeof(void *));
	*ud = ud;  // unique address
	if (luaL_newmetatable(L, "jsonnull")) {
        luaL_register(L, NULL, json_null);
	}
	lua_setmetatable(L, -2);
}

static void push_cjson_object(lua_State *L, cJSON *item) {
    switch (item->type) {
        case cJSON_False: lua_pushboolean(L, 0); break;
        case cJSON_True: lua_pushboolean(L, 1); break;
        case cJSON_NULL: lua_pushjsonnull(L); break;
        case cJSON_Number: lua_pushnumber(L, item->valuedouble); break;
        case cJSON_String: lua_pushstring(L, item->valuestring); break;
        case cJSON_Array: {
            lua_newtable(L);
            int i = 1;
            cJSON *child = item->child;
            while (child) {
                push_cjson_object(L, child);
                lua_rawseti(L, -2, i++);
                child = child->next;
            }
            break;
        }
        case cJSON_Object: {
            lua_newtable(L);
            cJSON *child = item->child;
            while (child) {
                lua_pushstring(L, child->string);
                push_cjson_object(L, child);
                lua_settable(L, -3);
                child = child->next;
            }
            break;
        }
        default:
            lua_pushnil(L);
    }
}

void *check_json_null(lua_State *L, int index) {
	if (lua_type(L, index) == LUA_TUSERDATA) {
		if (lua_getmetatable(L, index)) {
			luaL_getmetatable(L, "jsonnull");
			int is_equal = lua_rawequal(L, -1, -2);
			lua_pop(L, 2);
			if (is_equal) {
				return lua_touserdata(L, index);
			}
		}
	}
	return NULL;
}

static cJSON* lua_to_cjson(lua_State *L, int index) {
    void *null_obj = check_json_null(L, index);
    if (null_obj) return cJSON_CreateNull();
    int type = lua_type(L, index);
    switch(type) {
        case LUA_TBOOLEAN:
            return lua_toboolean(L, index) ? cJSON_CreateTrue() : cJSON_CreateFalse();
        case LUA_TNUMBER:
            return cJSON_CreateNumber(lua_tonumber(L, index));
        case LUA_TSTRING:
            return cJSON_CreateString(lua_tostring(L, index));
        case LUA_TTABLE: {
            // Decide if array or object by checking keys
            bool is_array = true;
            int n = lua_rawlen(L, index);
            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                if (lua_type(L, -2) != LUA_TNUMBER) {
                    is_array = false;
                }
                lua_pop(L, 1);
            }
            cJSON *json = is_array ? cJSON_CreateArray() : cJSON_CreateObject();

            lua_pushnil(L);
            while (lua_next(L, index) != 0) {
                cJSON *child = lua_to_cjson(L, lua_gettop(L));
                if (is_array) {
                    cJSON_AddItemToArray(json, child);
                } else {
                    if (lua_type(L, -2) == LUA_TSTRING) {
                        cJSON_AddItemToObject(json, lua_tostring(L, -2), child);
                    } else {
                        // non-string key: ignore or handle differently
                        cJSON_Delete(child);
                    }
                }
                lua_pop(L, 1);
            }
            return json;
        }
        default:
            return cJSON_CreateNull();
    }
}

static int lua_decode(lua_State *L) {
    const char *json_str = luaL_checkstring(L, 1);
    const char *error_ptr;
    cJSON *json = cJSON_ParseWithOpts(json_str, &error_ptr, 1);
    if (!json) {
        return luaL_error(L, "Parse error at: %s", error_ptr);
    }
    push_cjson_object(L, json);
    cJSON_Delete(json);
    return 1;
}

static int lua_encode(lua_State *L) {
    cJSON *json = lua_to_cjson(L, 1);
    bool unformatted = false; if(!lua_isnone(L, 2)) unformatted = lua_toboolean(L, 2);
    if (!json) {
        lua_pushnil(L);
        return 1;
    }
    char *json_str;
    if(unformatted) json_str = cJSON_PrintUnformatted(json);
    else json_str = cJSON_Print(json);
    cJSON_Delete(json);
    lua_pushstring(L, json_str);
    free(json_str);
    return 1;
}

static int lua_minify(lua_State *L) {
    char *src = strdup(luaL_checkstring(L, 1));
    cJSON_Minify(src);
    lua_pushstring(L, src);
    free(src);
    return 1;
}

static int lua_cjsonver(lua_State *L) {
    lua_pushstring(L, cJSON_Version());
    return 1;
}

static int lua_isnull(lua_State *L) {
	lua_pushboolean(L, check_json_null(L, 1) != NULL);
	return 1;
}

static const luaL_Reg json_lib[] = {
    {"decode", lua_decode},
    {"parse", lua_decode},
    {"encode", lua_encode},
    {"stringify", lua_encode},
    {"minify", lua_minify},
    {"isnull", lua_isnull},
    {"version", lua_cjsonver},
    {NULL, NULL}
};

LUALIB_API int luaL_openjson(lua_State *L) {
	luaL_register(L, "json", json_lib);
    lua_pushjsonnull(L);
	lua_setfield(L, -2, "null");
    return 1;
}