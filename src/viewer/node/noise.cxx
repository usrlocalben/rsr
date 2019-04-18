#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlg/rmlg_noise.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public IValue {
public:
	using VarDefList = std::vector<std::pair<std::string, std::string>>;

	Impl(std::string_view id, InputList inputs, float x, float y, float z)
		:IValue(id, std::move(inputs)), x_(x), y_(y), z_(z) {}

	// NodeBase
	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
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

	void Reset() override {
		cache_ = {}; }

	void AddDeps() override {
		IValue::AddDeps();
		AddDep(xNode_);
		AddDep(yNode_);
		AddDep(zNode_); }

	// IValue
	NamedValue Eval(std::string_view name) override {
		if (cache_.has_value()) {
			return cache_.value(); }

		float x{x_};
		if (xNode_ != nullptr) { x = xNode_->Eval(xSlot_).as_float(); }
		float y{y_};
		if (yNode_ != nullptr) { y = yNode_->Eval(ySlot_).as_float(); }
		float z{z_};
		if (zNode_ != nullptr) { z = zNode_->Eval(zSlot_).as_float(); }

		auto result = rmlg::PINoise(x, y, z);
		cache_ = NamedValue{ static_cast<float>(result) };
		return cache_.value(); }

private:
	std::optional<NamedValue> cache_{};
	float x_{0};
	float y_{0};
	float z_{0};
	IValue* xNode_{nullptr};
	IValue* yNode_{nullptr};
	IValue* zNode_{nullptr};
	std::string xSlot_{};
	std::string ySlot_{};
	std::string zSlot_{}; };


class Compiler final : public NodeCompiler {
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

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), x, y, z); }};


Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register("$noise", &compiler);
}} init{};


}  // namespace
}  // namespace rqdq
