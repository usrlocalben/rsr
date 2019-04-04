#pragma once
#include <memory>
#include <string>
#include <string_view>
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
	Integer,
	Real,
	Vec2,
	Vec3,
	Vec4 };


struct ValueTypeSerializer {
	static ValueType Deserialize(std::string_view data);
	static std::string_view Serialize(ValueType value); };


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
class ValuesBase : public NodeBase {
public:
	ValuesBase(std::string_view id, InputList inputs)
		:NodeBase(id, std::move(inputs)) {}

	virtual NamedValue Get(std::string_view name) = 0; };


class MultiValueNode : public ValuesBase {
public:
	MultiValueNode(std::string_view id, InputList inputs)
		:ValuesBase(id, std::move(inputs)) {}

	template<typename T>
	void Upsert(const std::string& name, const T value) {
		db_[name] = NamedValue{ name, value }; }

	NamedValue Get(std::string_view name) override {
		thread_local std::string tmp;
		tmp.assign(name);  // xxx yuck
		auto search = db_.find(tmp);
		return search == db_.end() ? notFoundValue_ : search->second; }

private:
	std::unordered_map<std::string, NamedValue> db_{};
	NamedValue notFoundValue_{ "__notfound__", int{0}}; };


class FloatNode : public ValuesBase {
public:
	FloatNode(std::string_view id, InputList inputs, float value)
		:ValuesBase(id, std::move(inputs)), value_(value) {}

	NamedValue Get(std::string_view name) override {
		thread_local std::string tmp;
		tmp.assign(name);
		float value = value_;
		if (xNode_ != nullptr) { value = xNode_->Get(xSlot_).as_float(); }
		return NamedValue{ "", value }; }

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "x") {
			xNode_ = static_cast<ValuesBase*>(other);
			xSlot_ = slot; }
		else {
			ValuesBase::Connect(attr, other, slot); }}

protected:
	void AddDeps() override {
		ValuesBase::AddDeps();
		AddDep(xNode_); }

private:
	float value_{};
	ValuesBase* xNode_{nullptr};
	std::string xSlot_{}; };


class Vec2Node : public ValuesBase {
public:
	Vec2Node(std::string_view id, InputList inputs, rmlv::vec2 value)
		:ValuesBase(id, std::move(inputs)), value_(value) {}

	NamedValue Get(std::string_view name) override {
		rmlv::vec2 value = value_;
		if (xNode_ != nullptr) { value.x = xNode_->Get(xSlot_).as_float(); }
		if (yNode_ != nullptr) { value.y = yNode_->Get(ySlot_).as_float(); }
		if (name == "x") { return NamedValue{ "x", value.x }; }
		if (name == "y") { return NamedValue{ "y", value.y }; }
		return NamedValue{ "", value }; }

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "x") {
			xNode_ = static_cast<ValuesBase*>(other);
			xSlot_ = slot; }
		if (attr == "y") {
			yNode_ = static_cast<ValuesBase*>(other);
			ySlot_ = slot; }
		else {
			ValuesBase::Connect(attr, other, slot); }}

protected:
	void AddDeps() override {
		ValuesBase::AddDeps();
		AddDep(xNode_);
		AddDep(yNode_); }

private:
	rmlv::vec2 value_{};
	ValuesBase* xNode_{nullptr};
	ValuesBase* yNode_{nullptr};
	std::string xSlot_{};
	std::string ySlot_{}; };


class Vec3Node : public ValuesBase {
public:
	Vec3Node(std::string_view id, InputList inputs, rmlv::vec3 value)
		:ValuesBase(id, std::move(inputs)), value_(value) {}

	NamedValue Get(std::string_view name) override {
		rmlv::vec3 value = value_;
		if (xNode_ != nullptr) { value.x = xNode_->Get(xSlot_).as_float(); }
		if (yNode_ != nullptr) { value.y = yNode_->Get(ySlot_).as_float(); }
		if (zNode_ != nullptr) { value.z = zNode_->Get(zSlot_).as_float(); }
		if (name == "x") { return NamedValue{ "x", value.x }; }
		if (name == "y") { return NamedValue{ "y", value.y }; }
		if (name == "z") { return NamedValue{ "z", value.z }; }
		if (name == "xy") { return NamedValue{ "xy", value.xy() }; }
		return NamedValue{ "", value }; }

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "x") {
			xNode_ = static_cast<ValuesBase*>(other);
			xSlot_ = slot; }
		if (attr == "y") {
			yNode_ = static_cast<ValuesBase*>(other);
			ySlot_ = slot; }
		if (attr == "z") {
			zNode_ = static_cast<ValuesBase*>(other);
			zSlot_ = slot; }
		else {
			ValuesBase::Connect(attr, other, slot); }}

protected:
	void AddDeps() override {
		ValuesBase::AddDeps();
		AddDep(xNode_);
		AddDep(yNode_);
		AddDep(zNode_); }

private:
	rmlv::vec3 value_;
	ValuesBase* xNode_{nullptr};
	ValuesBase* yNode_{nullptr};
	ValuesBase* zNode_{nullptr};
	std::string xSlot_{};
	std::string ySlot_{};
	std::string zSlot_{}; };


}  // namespace rqv
}  // namespace rqdq
