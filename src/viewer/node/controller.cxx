#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <mutex>

#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_value.hxx"
#include "src/viewer/node/i_controller.hxx"

#include "3rdparty/lua/lua.hpp"

namespace rqdq {
namespace {

using namespace rqv;
using vec2 = rmlv::vec2;
using vec3 = rmlv::vec3;
using vec4 = rmlv::vec4;

inline
void lua_swap(lua_State *L) {
	lua_insert(L, -2); }


/**
 * game controller script Lua
 */
class LuaController : public IValue, public IController {
	lua_State* const L;
	const std::string varName_;
	std::mutex mutex_;

public:
	LuaController(std::string_view id, InputList inputs, lua_State* L, std::string vn) :
		IValue(id, std::move(inputs)),
		IController(),
		L(L),
		varName_(std::move(vn)) {}

	~LuaController() {
		lua_close(L); }

	// Base...
	void Reset() override {
		lua_getglobal(L, varName_.c_str());
		lua_getfield(L, -1, "Reset");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			return; }
		lua_swap(L);
		lua_pcall(L, 1, 0, 0); }

	// IController
	void KeyPress(int k) override {
		lua_getglobal(L, varName_.c_str());
		lua_getfield(L, -1, "KeyPress");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			return; }
		lua_swap(L);
		lua_pushnumber(L, k);
		lua_pcall(L, 2, 0, 0); }

	void BeforeFrame(float t) override {
		lua_getglobal(L, varName_.c_str());
		lua_getfield(L, -1, "BeforeFrame");
		if (!lua_isfunction(L, -1)) {
			lua_pop(L, 2);
			return; }
		lua_swap(L);
		lua_pushnumber(L, t);
		lua_pcall(L, 2, 0, 0); }

	// IValue
	auto Eval(std::string_view name) -> NamedValue override {
		std::lock_guard lock(mutex_);

		lua_getglobal(L, varName_.c_str());
		lua_getfield(L, -1, "Eval");
		lua_swap(L);
		std::string tmp{name};
		lua_pushstring(L, tmp.c_str());
		lua_pcall(L, 2, 1, 0);
		if (lua_isstring(L, -1)) {
			size_t len;
			auto dataBegin = lua_tolstring(L, -1, &len);
			std::string v(dataBegin, len);
			lua_pop(L, 1);
			return NamedValue{ v }; }
		else if (lua_istable(L, -1)) {
			float buf[16];
			auto cnt = static_cast<int>(luaL_len(L, -1));
			for (int i=0; i<cnt; ++i) {
				lua_rawgeti(L, -1, i+1);
				buf[i] = static_cast<float>(lua_tonumber(L, -1));
				lua_pop(L, 1); }
			lua_pop(L, 1);
			if (cnt == 1) {
				return NamedValue{ buf[0] }; }
			else if (cnt == 2) {
				return NamedValue{ rmlv::vec2{ buf[0], buf[1] } }; }
			else if (cnt == 3) {
				return NamedValue{ rmlv::vec3{ buf[0], buf[1], buf[2] } }; }
			else if (cnt >= 4) {
				return NamedValue{ rmlv::vec4{ buf[0], buf[1], buf[2], buf[3] } }; }
			else {
				std::cerr << "eval: lua gave a table, but it looks empty\n";
				return NamedValue{ 0.0F }; }}
		else {
			std::cerr << "unknown lua eval result!\n";
			return NamedValue{ 0.0F }; }}};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;

		std::string name{"controller:controller"};
		if (auto jv = jv_find(data_, "instance", JSON_STRING)) {
			name.assign(jv->toString()); }

		auto segs = rclt::Split(name, ':');
		std::string fileName = std::string{"data\\"} + segs[0] + ".lua";
		std::string varName = segs[1];

		auto L = luaL_newstate();
		luaL_openlibs(L);
		if (luaL_dofile(L, fileName.c_str())) {
			throw std::runtime_error(lua_tostring(L, -1)); }

		out_ = std::make_shared<LuaController>(id_, std::move(inputs_), L, varName); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$controller", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
