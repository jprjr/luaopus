#include "luaopus_internal.h"
#include <opus/opus.h>
#include <assert.h>

/* recommendation from opus header is 4000 bytes */
#define MAX_PACKET 4000

/* max sample rate: 48000
 * max channels: 2
 * max ms: 120ms (5760 frames for 48kHz)
 * this may be overkill (I don't think the encoder interface
 * lets you encode more than 60ms at a time) */
#define MAX_SAMPLES 5760 * 2

const char * const luaopus_encoder_mt = "OpusEncoder";

struct luaopus_encoder_s {
    OpusEncoder *encoder;

    /* buffer for storing the encoded Opus packet
     * before passing to Lua */
    unsigned char buffer[MAX_PACKET];

    /* buffer for storing audio samples from Lua
     * before sending to Opus.
     * using float storage since that can
     * also encapsulate int16 */
    float pcm_float[MAX_SAMPLES];

    /* will point to pcm_float, so we use the same memory
     * area for floats and ints */
    opus_int16 *pcm_int16;
    int channels;

    /* stores a reference to the encoder userdata so it doesn't get
     * garbage-collected */
    int encoder_ref;
};

typedef struct luaopus_encoder_s luaopus_encoder;

static int
luaopus_OpusEncoder(lua_State *L) {
    luaopus_encoder *u = NULL;

    u = lua_newuserdata(L,sizeof(luaopus_encoder));
    if(u == NULL) {
        return luaL_error(L,"out of memory");
    }

    /* get a table to associate encoder reference with */
    lua_newtable(L);

    /* max channels for an encoder is 2 */
    /* we'll just always allocate the max */
    u->encoder = lua_newuserdata(L,opus_encoder_get_size(2));
    if(u->encoder == NULL) {
        return luaL_error(L,"out of memory");
    }
    u->encoder_ref = luaL_ref(L,-2);

    u->pcm_int16 = (opus_int16 *)u->pcm_float;

    lua_setuservalue(L,-2);

    luaL_setmetatable(L,luaopus_encoder_mt);
    return 1;
}

static int
luaopus_OpusEncoder_delete(lua_State *L) {
    luaopus_encoder *u = NULL;

    u = luaL_checkudata(L,1,luaopus_encoder_mt);
    lua_getuservalue(L,1);

    if(u->encoder_ref != LUA_NOREF) {
        luaL_unref(L,-1,u->encoder_ref);
        u->encoder_ref = LUA_NOREF;
        u->encoder = NULL;
    }

    return 0;
}

static int
luaopus_encoder_init(lua_State *L) {
    luaopus_encoder *u = NULL;
    opus_int32 Fs = 0;
    int channels = 0;
    int application = 0;
    int result = 0;

    u = luaL_checkudata(L,1,luaopus_encoder_mt);
    Fs = luaL_checkinteger(L,2);
    channels = luaL_checkinteger(L,3);
    application = luaL_checkinteger(L,4);
    result = opus_encoder_init(u->encoder,Fs,channels,application);

    if(result != OPUS_OK) {
        lua_pushnil(L);
        lua_pushinteger(L,result);
        return 2;
    }
    u->channels = channels;
    lua_pushboolean(L,1);
    return 1;
}

static int
luaopus_encode(lua_State *L) {
    luaopus_encoder *u = NULL;
    unsigned int frame_size = 0;
    unsigned int frames = 0;
    unsigned int f = 0;
    int bytes = 0;

    u = luaL_checkudata(L,1,luaopus_encoder_mt);
    frames = lua_rawlen(L,2);
    frame_size = frames / u->channels;

    while(f<frames) {
        lua_rawgeti(L,2,f+1);
        u->pcm_int16[f] = lua_tointeger(L,-1);
        lua_pop(L,1);
        f++;
    }

    bytes = opus_encode(u->encoder,
      u->pcm_int16,
      (int)frame_size,
      u->buffer,
      MAX_PACKET);

    if(bytes < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,bytes);
        return 2;
    }

    lua_pushlstring(L,(const char *)u->buffer,bytes);
    return 1;
}

static int
luaopus_encode_float(lua_State *L) {
    luaopus_encoder *u = NULL;
    unsigned int frame_size = 0;
    unsigned int frames = 0;
    unsigned int f = 0;
    int bytes = 0;

    u = luaL_checkudata(L,1,luaopus_encoder_mt);
    frames = lua_rawlen(L,2);
    frame_size = frames / u->channels;

    while(f<frames) {
        lua_rawgeti(L,2,f+1);
        u->pcm_float[f] = lua_tonumber(L,-1);
        lua_pop(L,1);
        f++;
    }

    bytes = opus_encode_float(u->encoder,
      u->pcm_float,
      (int)frame_size,
      u->buffer,
      MAX_PACKET);

    if(bytes < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,bytes);
        return 2;
    }

    lua_pushlstring(L,(const char *)u->buffer,bytes);
    return 1;
}

#define LUAOPUS_ENCODER_SET_INTEGER(f) LUAOPUS_CTL_SET_INTEGER(encoder,f)
#define LUAOPUS_ENCODER_GET_INTEGER(f) LUAOPUS_CTL_GET_INTEGER(encoder,f)
#define LUAOPUS_ENCODER_SET_UINTEGER(f) LUAOPUS_CTL_SET_UINTEGER(encoder,f)
#define LUAOPUS_ENCODER_GET_UINTEGER(f) LUAOPUS_CTL_GET_UINTEGER(encoder,f)
#define LUAOPUS_ENCODER_SET_BOOLEAN(f) LUAOPUS_CTL_SET_BOOLEAN(encoder,f)
#define LUAOPUS_ENCODER_GET_BOOLEAN(f) LUAOPUS_CTL_GET_BOOLEAN(encoder,f)

#define ctl_set(f) "opus_encoder_ctl_set_" f
#define CTL_SET(f) luaopus_encoder_ctl_set_ ## f
#define ctl_get(f) "opus_encoder_ctl_get_" f
#define CTL_GET(f) luaopus_encoder_ctl_get_ ## f

#define ctl_get_short(f) { "opus_encoder_ctl_get_" f , "get_" f }
#define ctl_set_short(f) { "opus_encoder_ctl_set_" f , "set_" f }

LUAOPUS_CTL_RESET_STATE(encoder)
LUAOPUS_ENCODER_GET_UINTEGER(FINAL_RANGE)
LUAOPUS_ENCODER_GET_INTEGER(BANDWIDTH)
LUAOPUS_ENCODER_GET_INTEGER(SAMPLE_RATE)

/* phase inversion and dtx don't appear until opus >= 1.2 */
#ifdef OPUS_SET_PHASE_INVERSION_DISABLED
LUAOPUS_ENCODER_SET_BOOLEAN(PHASE_INVERSION_DISABLED)
#endif
#ifdef OPUS_GET_PHASE_INVERSION_DISABLED
LUAOPUS_ENCODER_GET_BOOLEAN(PHASE_INVERSION_DISABLED)
#endif
#ifdef OPUS_GET_IN_DTX
LUAOPUS_ENCODER_GET_BOOLEAN(IN_DTX)
#endif

LUAOPUS_ENCODER_SET_INTEGER(COMPLEXITY)
LUAOPUS_ENCODER_GET_INTEGER(COMPLEXITY)

LUAOPUS_ENCODER_SET_INTEGER(BITRATE)
LUAOPUS_ENCODER_GET_INTEGER(BITRATE)

LUAOPUS_ENCODER_SET_BOOLEAN(VBR)
LUAOPUS_ENCODER_GET_BOOLEAN(VBR)

LUAOPUS_ENCODER_SET_BOOLEAN(VBR_CONSTRAINT)
LUAOPUS_ENCODER_GET_BOOLEAN(VBR_CONSTRAINT)

LUAOPUS_ENCODER_SET_INTEGER(FORCE_CHANNELS)
LUAOPUS_ENCODER_GET_INTEGER(FORCE_CHANNELS)

LUAOPUS_ENCODER_SET_INTEGER(MAX_BANDWIDTH)
LUAOPUS_ENCODER_GET_INTEGER(MAX_BANDWIDTH)

LUAOPUS_ENCODER_SET_INTEGER(SIGNAL)
LUAOPUS_ENCODER_GET_INTEGER(SIGNAL)

LUAOPUS_ENCODER_SET_INTEGER(APPLICATION)
LUAOPUS_ENCODER_GET_INTEGER(APPLICATION)

LUAOPUS_ENCODER_GET_INTEGER(LOOKAHEAD)

LUAOPUS_ENCODER_SET_BOOLEAN(INBAND_FEC)
LUAOPUS_ENCODER_GET_BOOLEAN(INBAND_FEC)

LUAOPUS_ENCODER_SET_INTEGER(PACKET_LOSS_PERC)
LUAOPUS_ENCODER_GET_INTEGER(PACKET_LOSS_PERC)

LUAOPUS_ENCODER_SET_BOOLEAN(DTX)
LUAOPUS_ENCODER_GET_BOOLEAN(DTX)

LUAOPUS_ENCODER_SET_INTEGER(LSB_DEPTH)
LUAOPUS_ENCODER_GET_INTEGER(LSB_DEPTH)

#ifdef OPUS_SET_EXPERT_FRAME_DURATION
LUAOPUS_ENCODER_SET_INTEGER(EXPERT_FRAME_DURATION)
#endif
#ifdef OPUS_GET_EXPERT_FRAME_DURATION
LUAOPUS_ENCODER_GET_INTEGER(EXPERT_FRAME_DURATION)
#endif

#ifdef OPUS_SET_PREDICTION_DISABLED
LUAOPUS_ENCODER_SET_BOOLEAN(PREDICTION_DISABLED)
#endif
#ifdef OPUS_GET_PREDICTION_DISABLED
LUAOPUS_ENCODER_GET_BOOLEAN(PREDICTION_DISABLED)
#endif

static const struct luaL_Reg luaopus_encoder_functions[] = {
    { "OpusEncoder", luaopus_OpusEncoder },
    { "opus_encoder_init", luaopus_encoder_init },
    { "opus_encode", luaopus_encode },
    { "opus_encode_float", luaopus_encode_float },
    { "opus_encoder_ctl_reset_state", luaopus_encoder_ctl_reset_state },
    { ctl_get("final_range"), CTL_GET(FINAL_RANGE) },
    { ctl_get("bandwdth"), CTL_GET(BANDWIDTH) },
    { ctl_get("samplerate"), CTL_GET(SAMPLE_RATE) },
#ifdef OPUS_SET_PHASE_INVERSION_DISABLED
    { ctl_set("phase_inversion_disabled"), CTL_SET(PHASE_INVERSION_DISABLED) },
#endif
#ifdef OPUS_GET_PHASE_INVERSION_DISABLED
    { ctl_get("phase_inversion_disabled"), CTL_GET(PHASE_INVERSION_DISABLED) },
#endif
#ifdef OPUS_GET_IN_DTX
    { ctl_get("in_dtx"), CTL_GET(IN_DTX) },
#endif
    { ctl_set("complexity"), CTL_SET(COMPLEXITY) },
    { ctl_get("complexity"), CTL_GET(COMPLEXITY) },
    { ctl_set("bitrate"), CTL_SET(BITRATE) },
    { ctl_get("bitrate"), CTL_GET(BITRATE) },
    { ctl_set("vbr"), CTL_SET(VBR) },
    { ctl_get("vbr"), CTL_GET(VBR) },
    { ctl_set("vbr_constraint"), CTL_SET(VBR_CONSTRAINT) },
    { ctl_get("vbr_constraint"), CTL_GET(VBR_CONSTRAINT) },
    { ctl_set("force_channels"), CTL_SET(FORCE_CHANNELS) },
    { ctl_get("force_channels"), CTL_GET(FORCE_CHANNELS) },
    { ctl_set("max_bandwidth"), CTL_SET(MAX_BANDWIDTH) },
    { ctl_get("max_bandwidth"), CTL_GET(MAX_BANDWIDTH) },
    { ctl_set("signal"), CTL_SET(SIGNAL) },
    { ctl_get("signal"), CTL_GET(SIGNAL) },
    { ctl_set("signal"), CTL_SET(APPLICATION) },
    { ctl_get("signal"), CTL_GET(APPLICATION) },
    { ctl_get("lookahead"), CTL_GET(LOOKAHEAD) },
    { ctl_set("inband_fec"), CTL_SET(INBAND_FEC) },
    { ctl_get("inband_fec"), CTL_GET(INBAND_FEC) },
    { ctl_set("packet_loss_perc"), CTL_SET(PACKET_LOSS_PERC) },
    { ctl_get("packet_loss_perc"), CTL_GET(PACKET_LOSS_PERC) },
    { ctl_set("dtx"), CTL_SET(DTX) },
    { ctl_get("dtx"), CTL_GET(DTX) },
    { ctl_set("lsb_depth"), CTL_SET(LSB_DEPTH) },
    { ctl_get("lsb_depth"), CTL_GET(LSB_DEPTH) },
#ifdef OPUS_SET_EXPERT_FRAME_DURATION
    { ctl_set("expert_frame_duration"), CTL_SET(EXPERT_FRAME_DURATION) },
#endif
#ifdef OPUS_GET_EXPERT_FRAME_DURATION
    { ctl_get("expert_frame_duration"), CTL_GET(EXPERT_FRAME_DURATION) },
#endif
#ifdef OPUS_SET_PREDICTION_DISABLED
    { ctl_set("prediction_disabled"), CTL_SET(PREDICTION_DISABLED) },
#endif
#ifdef OPUS_GET_PREDICTION_DISABLED
    { ctl_get("prediction_disabled"), CTL_GET(PREDICTION_DISABLED) },
#endif
    { NULL, NULL },
};

static const luaopus_metamethods luaopus_encoder_metamethods[] = {
    { "opus_encoder_init", "init" },
    { "opus_encode", "encode" },
    { "opus_encode_float", "encode_float" },
    ctl_get_short("final_range"),
    ctl_get_short("bandwdth"),
    ctl_get_short("samplerate"),
    ctl_set_short("phase_inversion_disabled"),
    ctl_get_short("phase_inversion_disabled"),
    ctl_get_short("in_dtx"),
    ctl_set_short("complexity"),
    ctl_get_short("complexity"),
    ctl_set_short("bitrate"),
    ctl_get_short("bitrate"),
    ctl_set_short("vbr"),
    ctl_get_short("vbr"),
    ctl_set_short("vbr_constraint"),
    ctl_get_short("vbr_constraint"),
    ctl_set_short("force_channels"),
    ctl_get_short("force_channels"),
    ctl_set_short("max_bandwidth"),
    ctl_get_short("max_bandwidth"),
    ctl_set_short("signal"),
    ctl_get_short("signal"),
    ctl_set_short("application"),
    ctl_get_short("application"),
    ctl_get_short("lookahead"),
    ctl_set_short("inband_fec"),
    ctl_get_short("inband_fec"),
    ctl_set_short("packet_loss_perc"),
    ctl_get_short("packet_loss_perc"),
    ctl_set_short("dtx"),
    ctl_get_short("dtx"),
    ctl_set_short("lsb_depth"),
    ctl_get_short("lsb_depth"),
    ctl_set_short("expert_frame_duration"),
    ctl_get_short("expert_frame_duration"),
    ctl_set_short("prediction_disabled"),
    ctl_get_short("prediction_disabled"),
    { NULL, NULL },
};


LUAOPUS_PUBLIC
int luaopen_luaopus_encoder(lua_State *L) {
    const luaopus_metamethods *m = luaopus_encoder_metamethods;

    lua_newtable(L);

    luaL_setfuncs(L,luaopus_encoder_functions,0);

    luaL_newmetatable(L,luaopus_encoder_mt);

    lua_pushcclosure(L,luaopus_OpusEncoder_delete,0);
    lua_setfield(L,-2,"__gc");

    lua_newtable(L);
    while(m->name != NULL) {
        lua_getfield(L,-3,m->name);
        lua_setfield(L,-2,m->metaname);
        m++;
    }

    lua_setfield(L,-2,"__index");
    lua_pop(L,1);
    return 1;
}
