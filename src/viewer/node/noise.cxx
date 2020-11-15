#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlg/rmlg_noise.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_value.hxx"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl final : public IValue {

	std::optional<NamedValue> cache_{};
	IValue* coordNode_{nullptr};
	std::string coordSlot_{};

public:
	using IValue::IValue;

	// NodeBase
	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "coord") {
			coordNode_ = dynamic_cast<IValue*>(other);
			coordSlot_ = slot;
			if (coordNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IValue::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IValue::DisconnectAll();
		coordNode_ = nullptr; }

	void Reset() override {
		cache_ = {}; }

	void AddDeps() override {
		IValue::AddDeps(); }

	// IValue
	auto Eval(std::string_view name [[maybe_unused]]) -> NamedValue override {
		if (cache_.has_value()) {
			return cache_.value(); }

		auto coord = coordNode_->Eval(coordSlot_).as_vec3();
		auto result = rmlg::PINoise(coord.x, coord.y, coord.z);
		cache_ = NamedValue{ static_cast<float>(result) };
		return cache_.value(); }};


class Compiler final : public NodeCompiler {
	void Build() override {
		Input("coord", /*required=*/true);
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$noise", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
