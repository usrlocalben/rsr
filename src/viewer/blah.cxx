#include "3rdparty/lua/lua.hpp"

#include <iostream>

void bail(lua_State* L, char* msg);

int main() {
	lua_State *L;
	L = luaL_newstate();

	luaL_openlibs(L);
	luaL_loadfile(L, "script.lua");
	lua_pcall(L, 0, 0, 0);
	lua_close(L);
	return 0; }


void bail(lua_State* L, char* msg) {
	std::cerr << "\nFATAL ERROR:\n  " << msg << ": " << lua_tostring(L, -1) << "\n\n";
	std::exit(1); }
