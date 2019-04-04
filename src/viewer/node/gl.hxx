#pragma once
#include <iostream>
#include <string>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/camera.hxx"
#include "src/viewer/node/value.hxx"

namespace rqdq {
namespace rqv {

class GlNode : public NodeBase {
public:
	GlNode(std::string_view id, InputList inputs)
		:NodeBase(id, std::move(inputs)) {}

	virtual void Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) = 0; };


class GroupNode : public GlNode {
public:
	GroupNode(std::string_view id, InputList inputs)
		:GlNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view) override;

	void Main() override;

	static void AllThen(rclmt::jobsys::Job* job, unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);

	void Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) override;

protected:
	void AddDeps() override {
		for (auto item : gls_) {
			AddDep(item); }}

protected:
	std::atomic<int> pcnt_;  // AllThen counter
	std::vector<GlNode*> gls_; };



class LayerNode : public GroupNode {
public:
	LayerNode(std::string_view id, InputList inputs)
		:GroupNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void Main() override;

	virtual rmlv::vec3 GetBackgroundColor() {
		auto color = rmlv::vec3{0.0f};
		if (colorNode_ != nullptr) {
			color = colorNode_->Get(colorSlot_).as_vec3(); }
		return color; }

	virtual void Render(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link);

protected:
	void AddDeps() override {
		GroupNode::AddDeps();
		AddDep(cameraNode_);
		AddDep(colorNode_); }

private:
	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(LayerNode::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<LayerNode*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

private:
	CameraNode* cameraNode_{nullptr};
	ValuesBase* colorNode_{nullptr};
	std::string colorSlot_{}; };


class LayerChooser : public LayerNode {
public:
	LayerChooser(std::string_view id, InputList inputs)
		:LayerNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void Main() override;

	rmlv::vec3 GetBackgroundColor() override;

	void Render(rglv::GL* dc, int width, int height, float aspect, rclmt::jobsys::Job *link) override;

protected:
	void AddDeps() override;

private:	
	LayerNode* GetSelectedLayer() const;

	std::vector<LayerNode*> layers_;
	ValuesBase* selectorNode_{nullptr};
	std::string selectorSlot_; };


}  // namespace rqv
}  // namespace rqdq
