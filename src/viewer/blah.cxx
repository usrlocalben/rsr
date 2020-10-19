#include "3rdparty/lua/lua.hpp"

#include <iostream>
#include <stdexcept>

void bail(lua_State* L, const char* msg);
void lua_swap(lua_State *L);

struct Float2 {
    float x;
    float y; };

auto operator<<(std::ostream& s, Float2 a) -> std::ostream&;

class GameControllerWrapper {

    lua_State *L;

public:
    GameControllerWrapper() {
        L = luaL_newstate();
        luaL_openlibs(L);
        if (luaL_dofile(L, "script.lua")) {
            bail(L, "dofile error"); }}

    ~GameControllerWrapper() {
        lua_close(L); }

    void Click(float x, float y) {
        lua_getglobal(L, "game");     // [ game ]
        lua_getfield(L, -1, "Click"); // [ game:Click, game ]
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:Click ]
        lua_pushnumber(L, x);         // [ x, game, game:Click ]
        lua_pushnumber(L, y);         // [ y, x, game, game:Click ]
        if (lua_pcall(L, 3, 0, 0)) {
            bail(L, "lua_pcall() failed"); }}

	void GetValue(const char* key, std::string& out) {
        lua_getglobal(L, "game");   
        lua_getfield(L, -1, "GetString");
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:GetVec2 ]
        lua_pushstring(L, key);
        if (lua_pcall(L, 2, 1, 0)) {
            bail(L, "lua_pcall() failed"); }
		size_t len;
		auto dataBegin = lua_tolstring(L, -1, &len);
		out.assign(dataBegin, len);
		lua_pop(L, 1); }

    void GetValue(const char* key, float& out) {
        lua_getglobal(L, "game");   
        lua_getfield(L, -1, "GetFloat");
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:GetVec2 ]
        lua_pushstring(L, key);
        if (lua_pcall(L, 2, 1, 0)) {
            bail(L, "lua_pcall() failed"); }
        out = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1); }

    void GetValue(const char* key, Float2& out) {
		std::cerr << "GV sp == " << lua_gettop(L);
        lua_getglobal(L, "game");   
        lua_getfield(L, -1, "GetVec2");
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:GetVec2 ]
        lua_pushstring(L, key);
        if (lua_pcall(L, 2, 1, 0)) {
            bail(L, "lua_pcall() failed"); }

		float buf[16];
		auto cnt = static_cast<int>(luaL_len(L, -1));
		std::cerr << "GV cnt is " << cnt << "\n";

		for (int i=0; i<cnt; ++i) {
			lua_rawgeti(L, -1, i+1);
			buf[i] = static_cast<float>(lua_tonumber(L, -1));
			lua_pop(L, 1); }
		lua_pop(L, 1);
        out.y = buf[0];
        out.x = buf[1];
		std::cerr << "GV sp == " << lua_gettop(L); }

    /*
    void GetValue(const char* key, Float3& out) {
        lua_getglobal(L, "game");   
        lua_getfield(L, -1, "GetVec3");
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:GetVec3 ]
        lua_pushstring(L, key);
        if (lua_pcall(L, 2, 3, 0)) {
            bail(L, "lua_pcall() failed"); }
        out.z = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1);
        out.y = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1);
        out.x = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1); }

    void GetValue(const char* key, Float4& out) {
        lua_getglobal(L, "game");   
        lua_getfield(L, -1, "GetVec4");
        if (!lua_isfunction(L, -1)) {
            throw std::runtime_error("expected game controller to have a Click method"); }
        lua_swap(L);                  // [ game, game:GetVec4 ]
        lua_pushstring(L, key);
        if (lua_pcall(L, 2, 4, 0)) {
            bail(L, "lua_pcall() failed"); }
        out.w = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1);
        out.z = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1);
        out.y = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1);
        out.x = static_cast<float>(lua_tonumber(L, -1));  lua_pop(L, 1); }
    */

    }; // GameControllerWrapper


int main() {
    try {
        GameControllerWrapper gc;
        gc.Click(0.4F, 0.4F);

        Float2 tmp;
        gc.GetValue("bar", tmp); std::cerr << "bar == " << tmp << "\n";
        gc.GetValue("baz", tmp); std::cerr << "baz == " << tmp << "\n";
		std::string xa;
		gc.GetValue("text", xa); std::cerr << "text == \"" << xa << "\"\n";

        return 0; }
    catch (const std::runtime_error& err) {
        std::cerr << "fatal: " << err.what();
        return 1; }}


void bail(lua_State* L, const char* msg) {
	std::cerr << "\nFATAL ERROR:\n  " << msg << ": " << lua_tostring(L, -1) << "\n\n";
	std::exit(1); }


inline
void lua_swap(lua_State *L) {
    lua_insert(L, -2); }


auto operator<<(std::ostream& s, Float2 a) -> std::ostream& {
    s << "<Float2 x=" << a.x << ", y=" << a.y << ">";
    return s; }
