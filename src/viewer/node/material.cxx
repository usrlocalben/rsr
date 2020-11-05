#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_texture.hxx"
#include "src/viewer/node/i_value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

class Impl : public IMaterial {

	// config
	const int programId_;
	const bool filter_;
	const bool alpha_;

	// inputs
	ITexture* textureNode0_{nullptr};
	ITexture* textureNode1_{nullptr};
	IValue* uv0Node_{nullptr};
	std::string uv0Slot_{};
	IValue* uv1Node_{nullptr};
	std::string uv1Slot_{};

public:
	Impl(std::string_view id, InputList inputs, int programId, bool filter, bool alpha) :
		IMaterial(id, std::move(inputs)),
		programId_(programId),
		filter_(filter),
		alpha_(alpha) {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "texture0") {
			textureNode0_ = dynamic_cast<ITexture*>(other);
			if (textureNode0_ == nullptr) {
				TYPE_ERROR(ITexture);
				return false; }
			return true; }
		if (attr == "texture1") {
			textureNode1_ = dynamic_cast<ITexture*>(other);
			if (textureNode1_ == nullptr) {
				TYPE_ERROR(ITexture);
				return false; }
			return true; }
		if (attr == "uv0") {
			uv0Node_ = dynamic_cast<IValue*>(other);
			uv0Slot_ = slot;
			if (uv0Node_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "uv1") {
			uv1Node_ = dynamic_cast<IValue*>(other);
			uv1Slot_ = slot;
			if (uv1Node_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IMaterial::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IMaterial::DisconnectAll();
		textureNode0_ = nullptr;
		textureNode1_ = nullptr;
		uv0Node_ = nullptr;
		uv1Node_ = nullptr; }

	void AddDeps() override {
		IMaterial::AddDeps();
		AddDep(textureNode0_);
		AddDep(textureNode1_); }

	void Main() override {
		jobsys::Job *postSetup = jobsys::make_job(jobsys::noop);
		AddLinksTo(postSetup);
		if (textureNode0_ == nullptr && textureNode1_ == nullptr) {
			run(postSetup); }
		else {
			if (textureNode0_ != nullptr) {
				textureNode0_->AddLink(AfterAll(postSetup));}
			if (textureNode1_ != nullptr) {
				textureNode1_->AddLink(AfterAll(postSetup));}
			if (textureNode0_ != nullptr) {
				textureNode0_->Run(); }
			if (textureNode1_ != nullptr) {
				textureNode1_->Run(); }}}

	void Apply(rglv::GL* _dc) override {
		auto& dc = *_dc;
		dc.UseProgram(int(programId_));
		if (textureNode0_ != nullptr) {
			auto& texture = textureNode0_->GetTexture();
			dc.BindTexture(0, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
		if (textureNode1_ != nullptr) {
			auto& texture = textureNode1_->GetTexture();
			dc.BindTexture(1, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
		dc.Enable(rglv::GL_CULL_FACE);
		if (alpha_) {
			dc.Enable(rglv::GL_BLEND); }
		else {
			dc.Disable(rglv::GL_BLEND); }

		if (uv0Node_ || uv1Node_) {
			auto [id, ptr] = dc.AllocUniformBuffer();
			dc.UseUniforms(id);
			if (uv0Node_) {
				auto dst = static_cast<rmlv::vec4*>(ptr);
				dst[0] = uv0Node_->Eval(uv0Slot_).as_vec4(); }
			if (uv1Node_) {
				auto dst = static_cast<rmlv::vec4*>(ptr);
				dst[1] = uv1Node_->Eval(uv1Slot_).as_vec4(); }} }};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("texture0", /*required=*/false)) { return; }
		if (!Input("texture1", /*required=*/false)) { return; }
		if (!Input("uv0", /*required=*/false)) { return; }
		if (!Input("uv1", /*required=*/false)) { return; }

		int programId = 1;
		if (auto jv = jv_find(data_, "program", JSON_STRING)) {
			programId = ShaderProgramNameSerializer::Deserialize(jv->toString()); }

		auto filter = DataBool("filter", false);
		auto alpha = DataBool("alpha", false);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), programId, filter, alpha); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$material", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq

