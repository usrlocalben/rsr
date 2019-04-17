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
public:
	Impl(std::string_view id, InputList inputs, ShaderProgramId programId, bool filter)
		:IMaterial(id, std::move(inputs)), programId_(programId), filter_(filter) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
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
		if (attr == "u0") {
			uNode0_ = dynamic_cast<IValue*>(other);
			uSlot0_ = slot;
			if (uNode0_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "u1") {
			uNode1_ = dynamic_cast<IValue*>(other);
			uSlot1_ = slot;
			if (uNode1_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IMaterial::Connect(attr, other, slot); }

	void AddDeps() override {
		IMaterial::AddDeps();
		AddDep(textureNode0_);
		AddDep(textureNode1_);
		AddDep(uNode0_);
		AddDep(uNode1_); }

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
		dc.glUseProgram(int(programId_));
		if (textureNode0_ != nullptr) {
			auto& texture = textureNode0_->GetTexture();
			dc.glBindTexture(0, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
		if (textureNode1_ != nullptr) {
			auto& texture = textureNode1_->GetTexture();
			dc.glBindTexture(1, texture.buf.data(), texture.width, texture.height, texture.stride, filter_ ? 1 : 0); }
		dc.glEnable(rglv::GL_CULL_FACE);
		if (uNode0_ != nullptr) {
			dc.glColor(uNode0_->Eval(uSlot0_).as_vec3()); }
		if (uNode1_ != nullptr) {
			dc.glNormal(uNode1_->Eval(uSlot1_).as_vec3()); }
		// dc.vertex_input_uniform(VertexInputUniform{ sin(float(gt.elapsed()*3.0f)) * 0.5f + 0.5f });
		}

private:
	// config
	ShaderProgramId programId_;
	bool filter_;

	// inputs
	ITexture* textureNode0_{nullptr};
	ITexture* textureNode1_{nullptr};
	IValue* uNode0_{nullptr};
	std::string uSlot0_{};
	IValue* uNode1_{nullptr};
	std::string uSlot1_{}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("texture0", /*required=*/false)) { return; }
		if (!Input("texture1", /*required=*/false)) { return; }
		if (!Input("u0", /*required=*/false)) { return; }
		if (!Input("u1", /*required=*/false)) { return; }

		ShaderProgramId programId = ShaderProgramId::Default;
		if (auto jv = jv_find(data_, "program", JSON_STRING)) {
			programId = ShaderProgramNameSerializer::Deserialize(jv->toString()); }

		bool filter{false};
		if (auto jv = jv_find(data_, "filter", JSON_TRUE)) {
			filter = true; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), programId, filter); }};

Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register("$material", &compiler);
}} init{};


}  // namespace
}  // namespace rqdq

