#include "luaopus.h"
#include <assert.h>

static void
copydown(lua_State *L, const char *tablename) {
    lua_getglobal(L,"require");
    lua_pushstring(L,tablename);
    lua_call(L,1,1);

    /* copies keys from table on top of stack to table below */
    lua_pushnil(L);
    while(lua_next(L,-2) != 0) {
        /* -1 = value
         * -2 = key */
        lua_pushvalue(L,-2);
        lua_insert(L,-2);
        lua_settable(L,-5);
    }
    lua_pop(L,1);
}

LUAOPUS_PUBLIC
int luaopen_luaopus(lua_State *L) {
    lua_newtable(L);

    copydown(L,"luaopus.version");
    copydown(L,"luaopus.defines");
    copydown(L,"luaopus.encoder");
    copydown(L,"luaopus.decoder");

    return 1;
}
