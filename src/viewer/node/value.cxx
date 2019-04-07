#include "value.hxx"

#include <memory>
#include <stdexcept>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/viewer/compile.hxx"

namespace rqdq {
namespace rqv {

ValueType ValueTypeSerializer::Deserialize(std::string_view data) {
	     if (data == "integer") { return ValueType::Integer; }
	else if (data == "real")    { return ValueType::Real; }
	else if (data == "vec2")    { return ValueType::Vec2; }
	else if (data == "vec3")    { return ValueType::Vec3; }
	else if (data == "vec4")    { return ValueType::Vec4; }
	else {
		std::cerr << "invalid value type \"" << data << "\", using real" << std::endl;
		return ValueType::Real; }}


std::string_view ValueTypeSerializer::Serialize(ValueType item) {
	switch (item) {
	case ValueType::Integer: return "integer";
	case ValueType::Real:    return "real";
	case ValueType::Vec2:    return "vec2";
	case ValueType::Vec3:    return "vec3";
	case ValueType::Vec4:    return "vec4";
	default: throw std::runtime_error("refusing to serialize invalid ValueType"); }}

}  // namespace rqv

namespace {

using namespace rqv;

class FloatCompiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		float x{0.0F};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "x", JSON_STRING)) {
			inputs_.emplace_back("x", jv->toString()); }

		out_ = std::make_shared<FloatNode>(id_, std::move(inputs_), x); }};


class Vec2Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;

		float x{0}, y{0};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "x", JSON_STRING)) {
			inputs_.emplace_back("x", jv->toString()); }

		if (auto jv = jv_find(data_, "y", JSON_NUMBER)) {
			y = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "y", JSON_STRING)) {
			inputs_.emplace_back("y", jv->toString()); }

		out_ = std::make_shared<Vec2Node>(id_, std::move(inputs_), rmlv::vec2{x, y}); }};


class Vec3Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		float x{0}, y{0}, z{0};

		if (auto jv = jv_find(data_, "x", JSON_NUMBER)) {
			x = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "x", JSON_STRING)) {
			inputs_.emplace_back("x", jv->toString()); }

		if (auto jv = jv_find(data_, "y", JSON_NUMBER)) {
			y = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "y", JSON_STRING)) {
			inputs_.emplace_back("y", jv->toString()); }

		if (auto jv = jv_find(data_, "z", JSON_NUMBER)) {
			z = static_cast<float>(jv->toNumber()); }
		else if (auto jv = jv_find(data_, "z", JSON_STRING)) {
			inputs_.emplace_back("z", jv->toString()); }

		out_ = std::make_shared<Vec3Node>(id_, std::move(inputs_), rmlv::vec3{x, y, z}); }};

FloatCompiler floatCompiler{};
Vec2Compiler vec2Compiler{};
Vec3Compiler vec3Compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register(NodeInfo{
		"IValue",
		"ValuesBase",
		[](NodeBase* node) { return dynamic_cast<ValuesBase*>(node) != nullptr; },
		nullptr });
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$float",
		"Float",
		[](NodeBase* node) { return dynamic_cast<FloatNode*>(node) != nullptr; },
		&floatCompiler });
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$vec2",
		"Vec2",
		[](NodeBase* node) { return dynamic_cast<Vec2Node*>(node) != nullptr; },
		&vec2Compiler });
	NodeRegistry::GetInstance().Register(NodeInfo{
		"$vec3",
		"Vec3",
		[](NodeBase* node) { return dynamic_cast<Vec3Node*>(node) != nullptr; },
		&vec3Compiler });
}} init{};


}  // namespace
}  // namespace rqdq
