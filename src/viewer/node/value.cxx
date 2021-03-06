#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_value.hxx"

#include <memory>
#include <stdexcept>
#include <string_view>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class StringNode : public IValue {
	std::string value_{};
public:
	StringNode(std::string_view id, InputList inputs, std::string value) :
		IValue(id, std::move(inputs)),
		value_(std::move(value)) {}

	auto Eval(std::string_view name) -> NamedValue override {
		return NamedValue{ value_ }; }};


class FloatNode : public IValue {

	float value_{};
	IValue* xNode_{nullptr};
	std::string xSlot_{};

public:
	FloatNode(std::string_view id, InputList inputs, float value) :
		IValue(id, std::move(inputs)),
		value_(value) {}

	auto Eval(std::string_view name) -> NamedValue override {
		thread_local std::string tmp;
		tmp.assign(name);
		float value = value_;
		if (xNode_ != nullptr) { value = xNode_->Eval(xSlot_).as_float(); }
		return NamedValue{ value }; }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "x") {
			xNode_ = dynamic_cast<IValue*>(other);
			xSlot_ = slot;
			if (xNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IValue::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IValue::DisconnectAll();
		xNode_ = nullptr; }

protected:
	void AddDeps() override {
		IValue::AddDeps(); }};


class Vec2Node : public IValue {

	rmlv::vec2 value_{};
	IValue* xNode_{nullptr};
	IValue* yNode_{nullptr};
	std::string xSlot_{};
	std::string ySlot_{}; 

public:
	Vec2Node(std::string_view id, InputList inputs, rmlv::vec2 value) :
		IValue(id, std::move(inputs)),
		value_(value) {}

	auto Eval(std::string_view name) -> NamedValue override {
		rmlv::vec2 value = value_;
		if (xNode_ != nullptr) { value.x = xNode_->Eval(xSlot_).as_float(); }
		if (yNode_ != nullptr) { value.y = yNode_->Eval(ySlot_).as_float(); }
		if (name == "x") { return NamedValue{ value.x }; }
		if (name == "y") { return NamedValue{ value.y }; }
		return NamedValue{ value }; }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "x") {
			xNode_ = dynamic_cast<IValue*>(other);
			xSlot_ = slot;
			if (xNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "y") {
			yNode_ = dynamic_cast<IValue*>(other);
			ySlot_ = slot;
			if (yNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IValue::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IValue::DisconnectAll();
		xNode_ = nullptr;
		yNode_ = nullptr; }

protected:
	void AddDeps() override {
		IValue::AddDeps(); }};


class Vec3Node : public IValue {
	rmlv::vec3 value_;
	IValue* xNode_{nullptr};
	IValue* yNode_{nullptr};
	IValue* zNode_{nullptr};
	std::string xSlot_{};
	std::string ySlot_{};
	std::string zSlot_{};

public:
	Vec3Node(std::string_view id, InputList inputs, rmlv::vec3 value) :
		IValue(id, std::move(inputs)),
		value_(value) {}

	auto Eval(std::string_view name) -> NamedValue override {
		rmlv::vec3 value = value_;
		if (xNode_ != nullptr) { value.x = xNode_->Eval(xSlot_).as_float(); }
		if (yNode_ != nullptr) { value.y = yNode_->Eval(ySlot_).as_float(); }
		if (zNode_ != nullptr) { value.z = zNode_->Eval(zSlot_).as_float(); }
		if (name == "x") { return NamedValue{ value.x }; }
		if (name == "y") { return NamedValue{ value.y }; }
		if (name == "z") { return NamedValue{ value.z }; }
		if (name == "xy") { return NamedValue{ value.xy() }; }
		return NamedValue{ value }; }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "x") {
			xNode_ = dynamic_cast<IValue*>(other);
			xSlot_ = slot;
			if (xNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "y") {
			yNode_ = dynamic_cast<IValue*>(other);
			ySlot_ = slot;
			if (yNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "z") {
			zNode_ = dynamic_cast<IValue*>(other);
			zSlot_ = slot;
			if (zNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IValue::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IValue::DisconnectAll();
		xNode_ = nullptr;
		yNode_ = nullptr;
		zNode_ = nullptr; }

protected:
	void AddDeps() override {
		IValue::AddDeps(); }};


class StringCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		std::string data{"emptystring"};
		if (auto jv = jv_find(data_, "data", JSON_STRING)) {
			data = jv->toString(); }
		out_ = std::make_shared<StringNode>(id_, std::move(inputs_), std::move(data)); }};


class FloatCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		float x{0.0F};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else {
			Input("x", /*required=*/false); }

		out_ = std::make_shared<FloatNode>(id_, std::move(inputs_), x); }};


class Vec2Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;

		float x{0}, y{0};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else {
			Input("x", /*required=*/false); }

		if (auto jv = jv_find(data_, "y", JSON_NUMBER)) {
			y = static_cast<float>(jv->toNumber()); }
		else {
			Input("y", /*required=*/false); }

		out_ = std::make_shared<Vec2Node>(id_, std::move(inputs_), rmlv::vec2{x, y}); }};


class Vec3Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		float x{0}, y{0}, z{0};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else {
			Input("x", /*required=*/false); }

		if (auto jv = jv_find(data_, "y", JSON_NUMBER)) {
			y = static_cast<float>(jv->toNumber()); }
		else {
			Input("y", /*required=*/false); }

		if (auto jv = jv_find(data_, "z", JSON_NUMBER)) {
			z = static_cast<float>(jv->toNumber()); }
		else {
			Input("z", /*required=*/false); }

		out_ = std::make_shared<Vec3Node>(id_, std::move(inputs_), rmlv::vec3{x, y, z}); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$string", []() { return std::make_unique<StringCompiler>(); });
	NodeRegistry::GetInstance().Register("$float", []() { return std::make_unique<FloatCompiler>(); });
	NodeRegistry::GetInstance().Register("$vec2", []() { return std::make_unique<Vec2Compiler>(); });
	NodeRegistry::GetInstance().Register("$vec3", []() { return std::make_unique<Vec3Compiler>(); });
}} init{};


}  // close unnamed namespace
