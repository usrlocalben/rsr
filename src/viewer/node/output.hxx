#pragma once
#include <string>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

class OutputNode : public NodeBase {
public:
	OutputNode(std::string_view id, InputList inputs)
		:NodeBase(id, std::move(inputs)) {}

	void Reset() override {
		NodeBase::Reset();
		doubleBuffer_ = false;
		tileDim_ = rmlv::ivec2{8,8};
		width_ = 0;
		height_ = 0;
		outCanvas_ = nullptr; }

	virtual void SetOutputCanvas(rglr::TrueColorCanvas* canvas) {
		outCanvas_ = canvas;
		width_ = canvas->width();
		height_ = canvas->height(); }

	virtual void SetDoubleBuffer(bool enable) {
		doubleBuffer_ = enable; }

	virtual void SetTileDim(rmlv::ivec2 dim) {
		tileDim_ = dim; }

protected:
	rglr::TrueColorCanvas* outCanvas_{nullptr};
	int width_{0};
	int height_{0};
	bool doubleBuffer_{false};
	rmlv::ivec2 tileDim_{8, 8}; };


}  // namespace rqv
}  // namespace rqdq
