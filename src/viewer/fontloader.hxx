#pragma once

#include <iostream>
#include <string>

struct lua_State;

namespace rqdq {
namespace rqv {

struct GlyphInfo {
	char ch;
	int width;
	int x, y;
	int w, h;
	int ox, oy; };


class LuaFontLoader {

	struct lua_State* L;

public:
	LuaFontLoader(const char* fn);
	~LuaFontLoader();
	LuaFontLoader(const LuaFontLoader&) = delete;
	auto operator=(const LuaFontLoader&) -> LuaFontLoader& = delete;

	auto File() -> std::string;
	auto HeightInPx() -> int;
	auto AscenderHeightInPx() -> int;
	auto Length() -> int;
	auto TextureDim() -> int;
	auto Entry(int) -> GlyphInfo; };


}  // close package namespace
}  // close enterprise namespace


auto operator<<(std::ostream& s, rqdq::rqv::GlyphInfo a) -> std::ostream&;
