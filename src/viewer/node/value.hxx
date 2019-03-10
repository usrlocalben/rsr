#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

/**
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;
*/

enum class ValueType {
	Integer, Real, Vec2, Vec3, Vec4
	};

ValueType name_to_type(const std::string& name);


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
	std::string name;
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
struct ValuesBase : public NodeBase {
	ValuesBase(const std::string& name, const InputList& inputs) :NodeBase(name, inputs) {}
	virtual NamedValue get(const std::string& name) = 0; };


struct MultiValueNode : public ValuesBase {
	std::unordered_map<std::string, NamedValue> kv;
	NamedValue not_found_value{ "__notfound__", int{0}};

	MultiValueNode(const std::string& name, const InputList& inputs) :ValuesBase(name, inputs) {}

	template<typename T>
	void upsert(const std::string& name, const T value) {
		kv[name] = NamedValue{ name, value }; }

	NamedValue get(const std::string& name) override {
		auto search = kv.find(name);
		if (search != kv.end()) {
			return search->second; }
		else {
			return not_found_value; }}};


struct FloatNode : public ValuesBase {
	const float d_value;
	ValuesBase *x_node=nullptr;
	std::string x_slot;

	FloatNode(const std::string& name, const InputList& inputs, const float value) :ValuesBase(name, inputs), d_value(value) {}

	NamedValue get(const std::string& name) override {
		float value = d_value;
		if (x_node) { value = x_node->get(x_slot).as_float(); }
		return NamedValue{ "", value }; }

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override {
		if (attr == "x") {
			x_node = dynamic_cast<ValuesBase*>(other);
			x_slot = slot; }
		else {
			std::cout << "FloatNode(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << std::endl; } }

	std::vector<NodeBase*> deps() override {
		std::vector<NodeBase*> out;
		if (x_node) out.push_back(x_node);
		return out; }};


struct Vec2Node : public ValuesBase {
	const rmlv::vec2 d_value;
	ValuesBase *x_node=nullptr, *y_node=nullptr;
	std::string x_slot, y_slot;

	Vec2Node(const std::string& name, const InputList& inputs, const rmlv::vec2 value) :ValuesBase(name, inputs), d_value(value) {}

	NamedValue get(const std::string& name) override {
		rmlv::vec2 value = d_value;
		if (x_node) value.x = x_node->get(x_slot).as_float();
		if (y_node) value.y = y_node->get(y_slot).as_float();
		if (name == "x") return NamedValue{ "x", value.x };
		if (name == "y") return NamedValue{ "y", value.y };
		return NamedValue{ "", value }; }

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override {
		if (attr == "x") {
			x_node = dynamic_cast<ValuesBase*>(other);
			x_slot = slot; }
		if (attr == "y") {
			y_node = dynamic_cast<ValuesBase*>(other);
			y_slot = slot; }
		else {
			std::cout << "Vec2Node(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << std::endl; } }

	std::vector<NodeBase*> deps() override {
		std::vector<NodeBase*> out;
		if (x_node) out.push_back(x_node);
		if (y_node) out.push_back(y_node);
		return out; }};


struct Vec3Node : public ValuesBase {
	const rmlv::vec3 d_value;
	ValuesBase *x_node=nullptr, *y_node=nullptr, *z_node=nullptr;
	std::string x_slot, y_slot, z_slot;

	Vec3Node(const std::string& name, const InputList& inputs, const rmlv::vec3 value) :ValuesBase(name, inputs), d_value(value) {}

	NamedValue get(const std::string& name) override {
		rmlv::vec3 value = d_value;
		if (x_node) value.x = x_node->get(x_slot).as_float();
		if (y_node) value.y = y_node->get(y_slot).as_float();
		if (z_node) value.z = z_node->get(z_slot).as_float();
		if (name == "x") return NamedValue{ "x", value.x };
		if (name == "y") return NamedValue{ "y", value.y };
		if (name == "z") return NamedValue{ "z", value.z };
		if (name == "xy") return NamedValue{ "xy", value.xy() };
		return NamedValue{ "", value }; }

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override {
		if (attr == "x") {
			x_node = dynamic_cast<ValuesBase*>(other);
			x_slot = slot; }
		if (attr == "y") {
			y_node = dynamic_cast<ValuesBase*>(other);
			y_slot = slot; }
		if (attr == "z") {
			z_node = dynamic_cast<ValuesBase*>(other);
			z_slot = slot; }
		else {
			std::cout << "Vec3Node(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << std::endl; } }

	std::vector<NodeBase*> deps() override {
		std::vector<NodeBase*> out;
		if (x_node) out.push_back(x_node);
		if (y_node) out.push_back(y_node);
		if (z_node) out.push_back(z_node);
		return out; }};


}  // namespace rqv
}  // namespace rqdq
