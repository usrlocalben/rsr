#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

/**
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
*/

enum class ValueType {
	Integer,
	Real,
	Vec2,
	Vec3,
	Vec4 };


struct ValueTypeSerializer {
	static ValueType Deserialize(std::string_view data);
	static std::string_view Serialize(ValueType item); };


template <class... Fs>
struct overloaded;

template <class F0, class... Frest>
struct overloaded<F0, Frest...> : F0, overloaded<Frest...> {
	overloaded(F0 f0, Frest... rest) : F0(f0), overloaded<Frest...>(rest...) {}
	using F0::operator();
	using overloaded<Frest...>::operator();
	};

template <class F0>
struct overloaded<F0> : F0 {
	overloaded(F0 f0) : F0(f0) {}
	using F0::operator();
	};

template <class... Fs>
auto make_visitor(Fs... fs) {
	return overloaded<Fs...>(fs...); }


struct NamedValue {
	// std::string name;
	std::variant<int, float, rmlv::vec2, rmlv::vec3, rmlv::vec4> data;

	int as_int() {
		return std::visit(make_visitor(
			[](const int x) { return x; },
			[](const float x) { return int(x); },
			[](const rmlv::vec2 x) { return int(x.x); },
			[](const rmlv::vec3 x) { return int(x.x); },
			[](const rmlv::vec4 x) { return int(x.x); }
			), data); }

	float as_float() {
		return std::visit(make_visitor(
			[](const int x) { return float(x); },
			[](const float x) { return x; },
			[](const rmlv::vec2 x) { return x.x; },
			[](const rmlv::vec3 x) { return x.x; },
			[](const rmlv::vec4 x) { return x.x; }
			), data); }

	rmlv::vec2 as_vec2() {
		return std::visit(make_visitor(
			[](const int x) { return rmlv::vec2{ float(x) }; },
			[](const float x) { return rmlv::vec2{ x }; },
			[](const rmlv::vec2 x) { return x; },
			[](const rmlv::vec3 x) { return rmlv::vec2{ x.x, x.y }; },
			[](const rmlv::vec4 x) { return rmlv::vec2{ x.x, x.y }; }
			), data); }

	rmlv::vec3 as_vec3() {
		return std::visit(make_visitor(
			[](const int x) { return rmlv::vec3{ float(x) }; },
			[](const float x) { return rmlv::vec3{ x }; },
			[](const rmlv::vec2 x) { return rmlv::vec3{ x.x, x.y, 0 }; },
			[](const rmlv::vec3 x) { return x; },
			[](const rmlv::vec4 x) { return rmlv::vec3{ x.x, x.y, x.z }; }
			), data); }

	rmlv::vec4 as_vec4() {
		return std::visit(make_visitor(
			[](const int x) { return rmlv::vec4{ float(x) }; },
			[](const float x) { return rmlv::vec4{ x }; },
			[](const rmlv::vec2 x) { return rmlv::vec4{ x.x, x.y, 0, 0 }; },
			[](const rmlv::vec3 x) { return rmlv::vec4{ x.x, x.y, x.z, 0 }; },
			[](const rmlv::vec4 x) { return x; }
			), data); }
	};


/**
 * a node with named values as sources
 */
class IValue : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual auto Eval(std::string_view name) -> NamedValue = 0; };


}  // namespace rqv
}  // namespace rqdq
