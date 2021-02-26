#include "luaopus_internal.h"
#include <opus/opus_defines.h>

static int
luaopus_strerror(lua_State *L) {
    lua_pushstring(L,opus_strerror(luaL_checkinteger(L,1)));
    return 1;
}

static int
luaopus_get_version_string(lua_State *L) {
    lua_pushstring(L,opus_get_version_string());
    return 1;
}

LUAOPUS_PUBLIC
int luaopen_luaopus_defines(lua_State *L) {
    lua_newtable(L);

    luaopus_push_const(OPUS_OK);
    luaopus_push_const(OPUS_BAD_ARG);
    luaopus_push_const(OPUS_BUFFER_TOO_SMALL);
    luaopus_push_const(OPUS_INTERNAL_ERROR);
    luaopus_push_const(OPUS_INVALID_PACKET);
    luaopus_push_const(OPUS_UNIMPLEMENTED);
    luaopus_push_const(OPUS_INVALID_STATE);
    luaopus_push_const(OPUS_ALLOC_FAIL);
    luaopus_push_const(OPUS_AUTO);
    luaopus_push_const(OPUS_BITRATE_MAX);
    luaopus_push_const(OPUS_APPLICATION_VOIP);
    luaopus_push_const(OPUS_APPLICATION_AUDIO);
    luaopus_push_const(OPUS_APPLICATION_RESTRICTED_LOWDELAY);
    luaopus_push_const(OPUS_SIGNAL_VOICE);
    luaopus_push_const(OPUS_SIGNAL_MUSIC);
    luaopus_push_const(OPUS_BANDWIDTH_NARROWBAND);
    luaopus_push_const(OPUS_BANDWIDTH_MEDIUMBAND);
    luaopus_push_const(OPUS_BANDWIDTH_WIDEBAND);
    luaopus_push_const(OPUS_BANDWIDTH_SUPERWIDEBAND);
    luaopus_push_const(OPUS_BANDWIDTH_FULLBAND);
    luaopus_push_const(OPUS_FRAMESIZE_ARG);
    luaopus_push_const(OPUS_FRAMESIZE_2_5_MS);
    luaopus_push_const(OPUS_FRAMESIZE_5_MS);
    luaopus_push_const(OPUS_FRAMESIZE_10_MS);
    luaopus_push_const(OPUS_FRAMESIZE_20_MS);
    luaopus_push_const(OPUS_FRAMESIZE_40_MS);
    luaopus_push_const(OPUS_FRAMESIZE_60_MS);
    luaopus_push_const(OPUS_FRAMESIZE_80_MS);
    luaopus_push_const(OPUS_FRAMESIZE_100_MS);
    luaopus_push_const(OPUS_FRAMESIZE_120_MS);

    lua_pushcclosure(L,luaopus_get_version_string,0);
    lua_setfield(L,-2,"opus_get_version_string");

    lua_pushcclosure(L,luaopus_strerror,0);
    lua_setfield(L,-2,"opus_strerror");

    return 1;
}
