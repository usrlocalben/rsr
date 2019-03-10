#pragma once
#include <memory>
#include <optional>
#include <string>
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

struct GPUNode : public NodeBase {
	// internal
	GPU gpu;

	std::atomic<int> pcnt;  // all_then counter

	// inputs
	std::vector<LayerNode*> layers;

	// received
	std::optional<int> width;
	std::optional<int> height;
	std::optional<float> aspect;
	std::optional<rmlv::ivec2> tile_dim;

public:
	GPUNode(const std::string& name, const InputList& inputs);

	void connect(const std::string&, NodeBase*, const std::string&) override;
	std::vector<NodeBase*> deps() override;
	void reset() override;
	void main() override;

	void setDimensions(int x, int y) { width = x; height = y; }
	void setTileDimensions(rmlv::ivec2 tile_dim_) { tile_dim = tile_dim_; }
	void setAspect(float aspect_) { aspect = aspect_; }

	rclmt::jobsys::Job* draw() {
		return rclmt::jobsys::make_job(GPUNode::drawJmp, std::tuple{this}); }
	static void drawJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->drawImpl(); }
	void drawImpl();

	static void all_then(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data);

	rclmt::jobsys::Job* post() {
		return rclmt::jobsys::make_job(GPUNode::postJmp, std::tuple{this}); }
	static void postJmp(rclmt::jobsys::Job* job, const unsigned tid, std::tuple<GPUNode*>* data) {
		auto& [self] = *data;
		self->post(); }
	void postImpl() {} };


}  // namespace rqv
}  // namespace rqdq
