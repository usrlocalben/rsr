#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/rqv_node_base.hxx"
#include "src/viewer/rqv_node_camera.hxx"
#include "src/viewer/rqv_node_value.hxx"

#include <iostream>
#include <string>

namespace rqdq {
namespace rqv {

struct GlNode : public NodeBase {
	GlNode(const std::string& name, const InputList& inputs) :NodeBase(name, inputs) {}
	virtual void draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) = 0; };


struct GroupNode : public GlNode {
	std::atomic<int> pcnt;  // all_then counter
	std::vector<GlNode*> gls;

public:
	GroupNode(const std::string& name, const InputList& inputs) :GlNode(name, inputs) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override {
		std::vector<NodeBase*> out;
		for (auto dep : gls) {
			out.push_back(dep); }
		return out; }

	void main() override;

	static void all_then(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);

	virtual void draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth);};



struct LayerNode : public GroupNode {
	CameraNode* camera_node = nullptr;
	ValuesBase* color_node = nullptr;  std::string color_slot;

public:
	LayerNode(const std::string& name, const InputList& inputs) :GroupNode(name, inputs) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override {
		auto deps = GroupNode::deps();
		if (camera_node) deps.push_back(camera_node);
		if (color_node) deps.push_back(color_node);
		return deps;}

	void main() override;

	virtual rmlv::vec3 getBackgroundColor() {
		auto color = rmlv::vec3{0.0f};
		if (color_node) {
			color = color_node->get(color_slot).as_vec3(); }
		return color; }

	virtual void draw(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link);

	rclmt::jobsys::Job* post() {
		return rclmt::jobsys::make_job(LayerNode::postJmp, std::tuple{this}); }
	static void postJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<LayerNode*>* data) {
		auto& [self] = *data;
		self->post(); }
	void postImpl() {}

	}; // end class


struct LayerChooser : public LayerNode {
	std::vector<LayerNode*> layers;
	ValuesBase* selector_node = nullptr;
	std::string selector_slot;

	LayerChooser(const std::string& name, const InputList& inputs) :LayerNode(name, inputs) {}

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;
	void main() override;

	rmlv::vec3 getBackgroundColor() override;
	void draw(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link) override;

private:	
	LayerNode* getSelectedLayer() const;
	}; // close LayerChooser


}  // close package namespace
}  // close enterprise namespace
