#include <err.h>
#include <stdlib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int main()
{
    lua_State* L;
    L = luaL_newstate();
    luaL_openlibs(L);
    if(luaL_loadfile(L, "test.lua") != LUA_OK)
        errx(EXIT_FAILURE, "luaL_loadfile() failed");
    lua_pcall(L, 0, 0, 0);
    lua_getglobal(L, "hello");
    lua_pcall(L, 0, 0, 0);
    lua_close(L);
    return 0;
}
