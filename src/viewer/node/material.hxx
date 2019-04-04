#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

class TextureNode : public NodeBase {
public:
	TextureNode(std::string_view id, InputList inputs)
		:NodeBase(id, std::move(inputs)) {}

	virtual const rglr::Texture& GetTexture() = 0; };


class MaterialNode : public NodeBase {
public:
	MaterialNode(std::string_view id, InputList inputs, ShaderProgramId programId, bool filter)
		:NodeBase(id, std::move(inputs)), programId_(programId), filter_(filter) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Main() override {
		using namespace rclmt::jobsys;
		Job *postSetup = make_job(noop);
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

	virtual void Apply(rglv::GL*);

private:
	// config
	ShaderProgramId programId_;
	bool filter_;

	// inputs
	TextureNode* textureNode0_{nullptr};
	TextureNode* textureNode1_{nullptr};
	ValuesBase* uNode0_{nullptr};
	std::string uSlot0_{};
	ValuesBase* uNode1_{nullptr};
	std::string uSlot1_{}; };


class ImageNode final : public TextureNode {
public:
	ImageNode(std::string_view id, InputList inputs, rglr::Texture texture)
		:TextureNode(id, std::move(inputs)), texture_(std::move(texture)) {}

	const rglr::Texture& GetTexture() override {
		return texture_; };

private:
	rglr::Texture texture_; };


}  // namespace rqv
}  // namespace rqdq
