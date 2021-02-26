#include "luaopus.h"

#define LUAOPUS_CTL_RESET_STATE(t) \
static int \
luaopus_ ## t ## _ctl_reset_state(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    err = opus_## t ##_ctl(u->t, OPUS_RESET_STATE); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,1); \
    return 1; \
}

#define LUAOPUS_CTL_SET_INTEGER(t,f) \
static int \
luaopus_ ## t ## _ctl_set_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_int32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    x = lua_tointeger(L,2); \
    err = opus_## t ##_ctl(u->t, OPUS_SET_ ## f(x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,1); \
    return 1; \
}

#define LUAOPUS_CTL_GET_INTEGER(t,f) \
static int \
luaopus_ ## t ## _ctl_get_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_int32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    err = opus_## t ##_ctl(u->t, OPUS_GET_ ## f(&x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushinteger(L,x); \
    return 1; \
}

#define LUAOPUS_CTL_SET_UINTEGER(t,f) \
static int \
luaopus_ ## t ## _ctl_set_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_uint32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    x = lua_tointeger(L,2); \
    err = opus_## t ##_ctl(u->t, OPUS_SET_ ## f(x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,1); \
    return 1; \
}

#define LUAOPUS_CTL_GET_UINTEGER(t,f) \
static int \
luaopus_ ## t ## _ctl_get_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_uint32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    err = opus_## t ##_ctl(u->t, OPUS_GET_ ## f(&x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,x); \
    return 1; \
}

#define LUAOPUS_CTL_SET_BOOLEAN(t,f) \
static int \
luaopus_ ## t ## _ctl_set_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_int32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    x = lua_toboolean(L,2); \
    err = opus_## t ##_ctl(u->t, OPUS_SET_ ## f(x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,1); \
    return 1; \
}

#define LUAOPUS_CTL_GET_BOOLEAN(t,f) \
static int \
luaopus_ ## t ## _ctl_get_ ## f(lua_State *L) { \
    luaopus_ ## t *u = NULL; \
    opus_int32 x = 0; \
    int err = 0; \
    u = luaL_checkudata(L,1,luaopus_ ## t ## _mt); \
    err = opus_## t ##_ctl(u->t, OPUS_GET_ ## f(&x)); \
    if(err < 0) { \
        lua_pushnil(L); \
        lua_pushinteger(L,err); \
        return 2; \
    } \
    lua_pushboolean(L,x); \
    return 1; \
}

#if __GNUC__ > 4
#define LUAOPUS_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define LUAOPUS_PRIVATE
#endif

typedef struct luaopus_metamethods_s {
    const char *name;
    const char *metaname;
} luaopus_metamethods;


#if (!defined LUA_VERSION_NUM) || LUA_VERSION_NUM == 501
#define lua_setuservalue(L,i) lua_setfenv((L),(i))
#define lua_getuservalue(L,i) lua_getfenv((L),(i))
#define lua_rawlen(L,i) lua_objlen((L),(i))
#endif

#define luaopus_push_const(x) lua_pushinteger(L,x) ; lua_setfield(L,-2, #x)

#ifdef __cplusplus
extern "C" {
#endif


#if !defined(luaL_newlibtable) \
  && (!defined LUA_VERSION_NUM || LUA_VERSION_NUM==501)
LUAOPUS_PRIVATE
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

LUAOPUS_PRIVATE
void luaL_setmetatable(lua_State *L, const char *str);

LUAOPUS_PRIVATE
void *luaL_testudata (lua_State *L, int i, const char *tname);
#endif

#ifdef __cplusplus
}
#endif
