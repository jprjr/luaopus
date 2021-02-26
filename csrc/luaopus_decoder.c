#include "luaopus_internal.h"
#include <opus/opus.h>
#include <assert.h>

/* max sample rate: 48000
 * max channels: 2
 * max ms: 120ms
 * (5760 frames for 48kHz)
 * I think this may be overkill, that there's some limitation
 * around sample rate for packets >60ms? */
#define MAX_SAMPLES 5760 * 2

const char * const luaopus_decoder_mt = "OpusDecoder";

struct luaopus_decoder_s {
    OpusDecoder *decoder;

    /* buffer for storing audio samples from Opus
     * before sending to Lua
     * using float storage since that can
     * also encapsulate int16 */
    float pcm_float[MAX_SAMPLES];

    /* will point to pcm_float, so we use the same memory
     * area for floats and ints */
    opus_int16 *pcm_int16;
    int channels;

    /* stores a reference to the decoder userdata so it doesn't get
     * garbage-collected */
    int decoder_ref;
};

typedef struct luaopus_decoder_s luaopus_decoder;

static int
luaopus_OpusDecoder(lua_State *L) {
    luaopus_decoder *u = NULL;

    u = lua_newuserdata(L,sizeof(luaopus_decoder));
    if(u == NULL) {
        return luaL_error(L,"out of memory");
    }

    /* get a table to associate decoder reference with */
    lua_newtable(L);

    /* max channels for an decoder is 2 */
    /* we'll just always allocate the max */
    u->decoder = lua_newuserdata(L,opus_decoder_get_size(2));
    if(u->decoder == NULL) {
        return luaL_error(L,"out of memory");
    }
    u->decoder_ref = luaL_ref(L,-2);

    u->pcm_int16 = (opus_int16 *)u->pcm_float;

    lua_setuservalue(L,-2);

    luaL_setmetatable(L,luaopus_decoder_mt);
    return 1;
}

static int
luaopus_OpusDecoder_delete(lua_State *L) {
    luaopus_decoder *u = NULL;

    u = luaL_checkudata(L,1,luaopus_decoder_mt);
    lua_getuservalue(L,1);

    if(u->decoder_ref != LUA_NOREF) {
        luaL_unref(L,-1,u->decoder_ref);
        u->decoder_ref = LUA_NOREF;
        u->decoder = NULL;
    }

    return 0;
}

static int
luaopus_decoder_init(lua_State *L) {
    luaopus_decoder *u = NULL;
    opus_int32 Fs = 0;
    int channels = 0;
    int result = 0;

    u = luaL_checkudata(L,1,luaopus_decoder_mt);
    Fs = luaL_checkinteger(L,2);
    channels = luaL_checkinteger(L,3);
    result = opus_decoder_init(u->decoder,Fs,channels);

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
luaopus_decode(lua_State *L) {
    luaopus_decoder *u = NULL;
    const unsigned char *data = NULL;
    size_t len = 0;
    int decode_fec = 0;
    int samples = 0;
    int i = 0;

    u = luaL_checkudata(L,1,luaopus_decoder_mt);
    data = (const unsigned char *)luaL_checklstring(L,2,&len);
    if(lua_isboolean(L,3)) {
        decode_fec = lua_toboolean(L,3);
    }

    samples = opus_decode(u->decoder,
      data,
      (opus_int32)len,
      u->pcm_int16,
      MAX_SAMPLES,
      decode_fec);

    if(samples < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,samples);
        return 2;
    }

    samples *= u->channels;

    lua_createtable(L, samples,  0);
    while(i<samples) {
        lua_pushinteger(L,u->pcm_int16[i]);
        lua_rawseti(L,-2,++i);
    }

    return 1;
}

static int
luaopus_decode_float(lua_State *L) {
    luaopus_decoder *u = NULL;
    const unsigned char *data = NULL;
    size_t len = 0;
    int decode_fec = 0;
    int samples = 0;
    int i = 0;

    u = luaL_checkudata(L,1,luaopus_decoder_mt);
    data = (const unsigned char *)luaL_checklstring(L,2,&len);
    if(lua_isboolean(L,3)) {
        decode_fec = lua_toboolean(L,3);
    }

    samples = opus_decode_float(u->decoder,
      data,
      (opus_int32)len,
      u->pcm_float,
      MAX_SAMPLES,
      decode_fec);

    if(samples < 0) {
        lua_pushnil(L);
        lua_pushinteger(L,samples);
        return 2;
    }

    samples *= u->channels;

    lua_createtable(L, samples,  0);
    while(i<samples) {
        lua_pushnumber(L,u->pcm_float[i]);
        lua_rawseti(L,-2,++i);
    }

    return 1;
}

static int
luaopus_packet_get_bandwidth(lua_State *L) {
    const unsigned char *data = NULL;
    size_t datalen = 0;
    int r = 0;
    data = (const unsigned char *)luaL_checklstring(L,1,&datalen);
    r = opus_packet_get_bandwidth(data);
    if(r == OPUS_INVALID_PACKET) {
        lua_pushnil(L);
        lua_pushinteger(L,r);
        return 2;
    }
    lua_pushinteger(L,r);
    return 1;
}

static int
luaopus_packet_get_samples_per_frame(lua_State *L) {
    const unsigned char *data = NULL;
    opus_int32 Fs = 0;
    size_t datalen = 0;

    data = (const unsigned char *)luaL_checklstring(L,1,&datalen);
    Fs = luaL_checkinteger(L,2);

    lua_pushinteger(L,
      opus_packet_get_samples_per_frame(data,Fs));
    return 1;
}

static int
luaopus_packet_get_nb_channels(lua_State *L) {
    const unsigned char *data = NULL;
    size_t datalen = 0;
    int r = 0;
    data = (const unsigned char *)luaL_checklstring(L,1,&datalen);
    r = opus_packet_get_nb_channels(data);
    if(r == OPUS_INVALID_PACKET) {
        lua_pushnil(L);
        lua_pushinteger(L,r);
        return 2;
    }
    lua_pushinteger(L,r);
    return 1;
}

static int
luaopus_packet_get_nb_frames(lua_State *L) {
    const unsigned char *data = NULL;
    size_t datalen = 0;
    int r = 0;
    data = (const unsigned char *)luaL_checklstring(L,1,&datalen);
    r = opus_packet_get_nb_frames(data,(opus_int32)datalen);
    if(r == OPUS_INVALID_PACKET || r == OPUS_BAD_ARG) {
        lua_pushnil(L);
        lua_pushinteger(L,r);
        return 2;
    }
    lua_pushinteger(L,r);
    return 1;
}

static int
luaopus_packet_get_nb_samples(lua_State *L) {
    const unsigned char *data = NULL;
    size_t datalen = 0;
    opus_int32 Fs = 0;
    int r = 0;

    data = (const unsigned char *)luaL_checklstring(L,1,&datalen);
    Fs = luaL_checkinteger(L,2);

    r = opus_packet_get_nb_samples(data,(opus_int32)datalen, Fs);
    if(r == OPUS_INVALID_PACKET || r == OPUS_BAD_ARG) {
        lua_pushnil(L);
        lua_pushinteger(L,r);
        return 2;
    }
    lua_pushinteger(L,r);
    return 1;
}

static int
luaopus_decoder_get_nb_samples(lua_State *L) {
    luaopus_decoder *u = NULL;
    const unsigned char *data = NULL;
    size_t datalen = 0;
    int r = 0;

    u = luaL_checkudata(L,1,luaopus_decoder_mt);
    data = (const unsigned char *)luaL_checklstring(L,2,&datalen);

    r = opus_decoder_get_nb_samples(u->decoder, data,(opus_int32)datalen);
    if(r == OPUS_INVALID_PACKET || r == OPUS_BAD_ARG) {
        lua_pushnil(L);
        lua_pushinteger(L,r);
        return 2;
    }
    lua_pushinteger(L,r);
    return 1;

    return 1;
}

static int
luaopus_pcm_soft_clip(lua_State *L) {
    float *pcm = NULL;
    int frame_size = 0;
    int channels = 0;
    int samples = 0;
    int i = 0;
    float *softclip_mem = NULL;

    if(!lua_istable(L,1)) {
        return luaL_error(L,"expected table of floats");
    }
    channels = luaL_checkinteger(L,2);

    samples = lua_rawlen(L,1);
    frame_size = samples / channels;

    pcm = lua_newuserdata(L,sizeof(float) * samples);
    if(pcm == NULL) {
        return luaL_error(L,"out of memory");
    }
    softclip_mem = lua_newuserdata(L,sizeof(float) * channels);
    if(softclip_mem == NULL) {
        return luaL_error(L,"out of memory");
    }

    i = samples;
    while(i) {
        lua_rawgeti(L,1,i);
        pcm[--i] = lua_tonumber(L,-1);
        lua_pop(L,1);
    }

    opus_pcm_soft_clip(pcm,frame_size,channels,softclip_mem);

    i=0;
    while(i<samples) {
        lua_pushnumber(L,pcm[i]);
        lua_rawseti(L,1,++i);
    }

    return 0;
}

#define LUAOPUS_DECODER_SET_INTEGER(f) LUAOPUS_CTL_SET_INTEGER(decoder,f)
#define LUAOPUS_DECODER_GET_INTEGER(f) LUAOPUS_CTL_GET_INTEGER(decoder,f)
#define LUAOPUS_DECODER_SET_UINTEGER(f) LUAOPUS_CTL_SET_UINTEGER(decoder,f)
#define LUAOPUS_DECODER_GET_UINTEGER(f) LUAOPUS_CTL_GET_UINTEGER(decoder,f)
#define LUAOPUS_DECODER_SET_BOOLEAN(f) LUAOPUS_CTL_SET_BOOLEAN(decoder,f)
#define LUAOPUS_DECODER_GET_BOOLEAN(f) LUAOPUS_CTL_GET_BOOLEAN(decoder,f)

#define ctl_set(f) "opus_decoder_ctl_set_" f
#define CTL_SET(f) luaopus_decoder_ctl_set_ ## f
#define ctl_get(f) "opus_decoder_ctl_get_" f
#define CTL_GET(f) luaopus_decoder_ctl_get_ ## f

#define ctl_get_short(f) { "opus_decoder_ctl_get_" f , "get_" f }
#define ctl_set_short(f) { "opus_decoder_ctl_set_" f , "set_" f }

LUAOPUS_CTL_RESET_STATE(decoder)
LUAOPUS_DECODER_GET_UINTEGER(FINAL_RANGE)
LUAOPUS_DECODER_GET_INTEGER(BANDWIDTH)
LUAOPUS_DECODER_GET_INTEGER(SAMPLE_RATE)
LUAOPUS_DECODER_SET_BOOLEAN(PHASE_INVERSION_DISABLED)
LUAOPUS_DECODER_GET_BOOLEAN(PHASE_INVERSION_DISABLED)
LUAOPUS_DECODER_GET_BOOLEAN(IN_DTX)

LUAOPUS_DECODER_SET_INTEGER(GAIN)
LUAOPUS_DECODER_GET_INTEGER(GAIN)
LUAOPUS_DECODER_GET_INTEGER(LAST_PACKET_DURATION)
LUAOPUS_DECODER_GET_INTEGER(PITCH)

static const struct luaL_Reg luaopus_decoder_functions[] = {
    { "OpusDecoder", luaopus_OpusDecoder },
    { "opus_decoder_init", luaopus_decoder_init },
    { "opus_decode", luaopus_decode },
    { "opus_decode_float", luaopus_decode_float },
    { "opus_decoder_ctl_reset_state", luaopus_decoder_ctl_reset_state },
    { "opus_packet_get_bandwidth", luaopus_packet_get_bandwidth },
    { "opus_packet_get_samples_per_frame", luaopus_packet_get_samples_per_frame },
    { "opus_packet_get_nb_channels", luaopus_packet_get_nb_channels },
    { "opus_packet_get_nb_frames", luaopus_packet_get_nb_frames },
    { "opus_packet_get_nb_samples", luaopus_packet_get_nb_samples },
    { "opus_decoder_get_nb_samples", luaopus_decoder_get_nb_samples },
    { "opus_pcm_soft_clip", luaopus_pcm_soft_clip },
    { ctl_get("final_range"), CTL_GET(FINAL_RANGE) },
    { ctl_get("bandwdth"), CTL_GET(BANDWIDTH) },
    { ctl_get("samplerate"), CTL_GET(SAMPLE_RATE) },
    { ctl_set("phase_inversion_disabled"), CTL_SET(PHASE_INVERSION_DISABLED) },
    { ctl_get("phase_inversion_disabled"), CTL_GET(PHASE_INVERSION_DISABLED) },
    { ctl_get("in_dtx"), CTL_GET(IN_DTX) },
    { ctl_get("gain"), CTL_GET(GAIN) },
    { ctl_set("gain"), CTL_SET(GAIN) },
    { ctl_get("last_packet_duration"), CTL_GET(LAST_PACKET_DURATION) },
    { ctl_get("pitch"), CTL_GET(PITCH) },
    { NULL, NULL },
};

static const luaopus_metamethods luaopus_decoder_metamethods[] = {
    { "opus_decoder_init", "init" },
    { "opus_decode", "decode" },
    { "opus_decode_float", "decode_float" },
    { "opus_deocder_get_nb_samples", "get_nb_samples" },
    ctl_get_short("final_range"),
    ctl_get_short("bandwdth"),
    ctl_get_short("samplerate"),
    ctl_set_short("phase_inversion_disabled"),
    ctl_get_short("phase_inversion_disabled"),
    ctl_get_short("in_dtx"),
    ctl_get_short("gain"),
    ctl_set_short("gain"),
    ctl_get_short("last_packet_duration"),
    ctl_get_short("pitch"),
    { NULL, NULL },
};

LUAOPUS_PUBLIC
int luaopen_luaopus_decoder(lua_State *L) {
    const luaopus_metamethods *m = luaopus_decoder_metamethods;

    lua_newtable(L);

    luaL_setfuncs(L,luaopus_decoder_functions,0);

    luaL_newmetatable(L,luaopus_decoder_mt);

    lua_pushcclosure(L,luaopus_OpusDecoder_delete,0);
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
