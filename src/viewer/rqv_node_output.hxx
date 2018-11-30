#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/rqv_node_base.hxx"

#include <string>

namespace rqdq {
namespace rqv {

struct OutputNode : public NodeBase {
	// received
	rglr::TrueColorCanvas * outcanvas = nullptr;
	int width, height;
	bool double_buffer;
	rmlv::ivec2 tile_dim;

	OutputNode(const std::string& name, const InputList& inputs) :NodeBase(name, inputs) {}

	void reset() override {
		NodeBase::reset();
		double_buffer = false;
		tile_dim = rmlv::ivec2{8,8};
		width = 0;
		height = 0;
		outcanvas = nullptr; }

	virtual void set_output_canvas(rglr::TrueColorCanvas *canvas) {
		outcanvas = canvas;
		width = canvas->width();
		height = canvas->height(); }

	virtual void set_double_buffer(bool enable) {
		double_buffer = enable; }

	virtual void set_tile_dim(const rmlv::ivec2 dim) {
		tile_dim = dim; }};

}  // close package namespace
}  // close enterprise namespace
