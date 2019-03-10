#pragma once
#include <string>
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

struct TextureNode : NodeBase {
	TextureNode(const std::string& name, const InputList& inputs) :NodeBase(name, inputs) {}
	virtual const rglr::Texture& getTexture() = 0; };


struct MaterialNode : NodeBase {
	// config
	const ShaderProgramId d_program;
	const bool d_filter;

	// inputs
	TextureNode* texture0_node = nullptr;
	TextureNode* texture1_node = nullptr;
	ValuesBase* u0_node = nullptr;  std::string u0_slot;
	ValuesBase* u1_node = nullptr;  std::string u1_slot;

	MaterialNode(
		const std::string& name,
		const InputList& inputs,
		const ShaderProgramId program,
		const bool filter
	) :NodeBase(name, inputs), d_program(program), d_filter(filter) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;

	void main() override {
		namespace jobsys = rclmt::jobsys;

		jobsys::Job *postSetup = jobsys::make_job(jobsys::noop);
		add_links_to(postSetup);
		if (texture0_node == nullptr && texture1_node == nullptr) {
			jobsys::run(postSetup); }
		else {
			if (texture0_node) {
				texture0_node->add_link(after_all(postSetup));}
			if (texture1_node) {
				texture1_node->add_link(after_all(postSetup));}
			if (texture0_node) {
				texture0_node->run(); }
			if (texture1_node) {
				texture1_node->run(); }}}

	virtual void apply(rglv::GL*);};


struct ImageNode : TextureNode {
	// config
	const rglr::Texture d_texture;

	ImageNode(
		const std::string& name,
		const InputList& inputs,
		const rglr::Texture texture
	) :TextureNode(name, inputs), d_texture(texture) {}

	virtual const rglr::Texture& getTexture() { return d_texture; };};


}  // namespace rqv
}  // namespace rqdq