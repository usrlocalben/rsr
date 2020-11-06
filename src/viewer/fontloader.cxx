#include "fontloader.hxx"

#include "3rdparty/lua/lua.hpp"

#include <iostream>
#include <stdexcept>

namespace rqdq {
namespace {

void bail(lua_State* L, const char* msg) {
	std::cerr << "\nFATAL ERROR:\n  " << msg << ": " << lua_tostring(L, -1) << "\n\n";
	std::exit(1); }

/*
inline
void lua_swap(lua_State *L) {
    lua_insert(L, -2); }
*/

}  // close unnamed namespace

namespace rqv {

LuaFontLoader::LuaFontLoader(const char* fn) {
	L = luaL_newstate();
	luaL_openlibs(L);
	if (luaL_dofile(L, fn)) {
		bail(L, "dofile error"); }}

LuaFontLoader::~LuaFontLoader() {
	lua_close(L); }

auto LuaFontLoader::File() -> std::string {
	lua_getfield(L, -1, "file");
	std::string tmp = lua_tostring(L, -1);
	lua_pop(L, 1);
	return tmp; }

auto LuaFontLoader::HeightInPx() -> int {
	lua_getfield(L, -1, "height");
	int tmp = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);
	return tmp; }

auto LuaFontLoader::AscenderHeightInPx() -> int {
	lua_getfield(L, -1, "metrics");
	lua_getfield(L, -1, "ascender");
	auto w = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 2);
	return w; }

auto LuaFontLoader::Length() -> int {
	lua_getfield(L, -1, "chars");
	auto cnt = static_cast<int>(luaL_len(L, -1));
	lua_pop(L, 1);
	return cnt; }

auto LuaFontLoader::TextureDim() -> int {
	lua_getfield(L, -1, "texture");
	lua_getfield(L, -1, "width");
	auto w = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 2);
	return w; }

auto LuaFontLoader::Entry(int i) -> GlyphInfo {
	GlyphInfo gi;
	lua_getfield(L, -1, "chars");
	lua_rawgeti(L, -1, i+1);

	lua_getfield(L, -1, "char");
	gi.ch = *lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "width");
	gi.width = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "x");
	gi.x = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "y");
	gi.y = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "w");
	gi.w = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "h");
	gi.h = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "ox");
	gi.ox = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "oy");
	gi.oy = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	lua_pop(L, 2);
	return gi; }


}  // close package namespace
}  // close enterprise namespace


auto operator<<(std::ostream& s, rqdq::rqv::GlyphInfo a) -> std::ostream& {
	s << "<Glyph ch=\"" << a.ch << "\"";
	s << " width=" << a.width;
	s << " x=" << a.x;
	s << " y=" << a.y;
	s << " w=" << a.w;
	s << " h=" << a.h;
	s << " ox=" << a.ox;
	s << " oy=" << a.oy;
	s << ">";
	return s; }

/*
int main() {
    try {
        LuaFontLoader lfl("c:\\users\\benjamin\\desktop\\cmu_serif_roman_48.lua");
		std::cerr << "file = \"" << lfl.File() << "\"\n";
		std::cerr << "height = " << lfl.HeightInPx() << " px\n";
		std::cerr << "density = " << lfl.Length() << " chars\n";
		std::cout << lfl.Entry(0) << "\n";
		std::cout << lfl.Entry(1) << "\n";
        return 0; }
    catch (const std::runtime_error& err) {
        std::cerr << "fatal: " << err.what();
        return 1; }}
*/
