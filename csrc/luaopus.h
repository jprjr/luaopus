#include <lua.h>
#include <lauxlib.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(_MSC_VER)
#define LUAOPUS_PUBLIC __declspec(dllexport)
#elif __GNUC__ > 4
#define LUAOPUS_PUBLIC __attribute__ ((visibility ("default")))
#else
#define LUAOPUS_PUBLIC
#endif

#ifdef __cplusplus
extern "C" {
#endif

LUAOPUS_PUBLIC
int luaopen_luaopus(lua_State *L);

LUAOPUS_PUBLIC
int luaopen_luaopus_encoder(lua_State *L);

LUAOPUS_PUBLIC
int luaopen_luaopus_decoder(lua_State *L);

LUAOPUS_PUBLIC
int luaopen_luaopus_defines(lua_State *L);

#ifdef __cplusplus
}
#endif
