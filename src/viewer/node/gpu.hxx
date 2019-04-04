#pragma once
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/camera.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

using GPU = rglv::GPU<rglv::BaseProgram, WireframeProgram, IQPostProgram, EnvmapProgram, AmyProgram, EnvmapXProgram>;

class GPUNode : public NodeBase {
public:
	GPUNode(std::string_view id, InputList inputs);

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Reset() override;

	void Main() override;

	void SetDimensions(int x, int y) {
		width_ = x; height_ = y; }

	void SetTileDimensions(rmlv::ivec2 tileDim) {
		tileDim_ = tileDim; }

	void SetAspect(float aspect) {
		aspect_ = aspect; }

	rclmt::jobsys::Job* Draw() {
		return rclmt::jobsys::make_job(GPUNode::DrawJmp, std::tuple{this}); }
	static void DrawJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->DrawImpl(); }
	void DrawImpl();

	static void AllThen(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);

	rclmt::jobsys::Job* Post() {
		return rclmt::jobsys::make_job(GPUNode::PostJmp, std::tuple{this}); }
	static void PostJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->Post(); }
	void PostImpl() {}

	GPU& get_gpu() {
		return gpu; }

private:
	// internal
	GPU gpu;

	std::atomic<int> pcnt_;  // all_then counter

	// inputs
	std::vector<LayerNode*> layers_;

	// received
	std::optional<int> width_;
	std::optional<int> height_;
	std::optional<float> aspect_;
	std::optional<rmlv::ivec2> tileDim_; };


}  // namespace rqv
}  // namespace rqdq
