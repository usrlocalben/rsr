#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/value.hxx"

namespace rqdq {
namespace rqv {

const int DEPTH_FORK_UNTIL = 100;


class RepeatOp final : public GlNode {
public:
	RepeatOp(std::string_view id, InputList inputs, int cnt, rmlv::vec3 translate, rmlv::vec3 rotate, rmlv::vec3 scale)
		:GlNode(id, std::move(inputs)),
		cnt_(cnt),
		translateFixed_(translate),
		rotateFixed_(rotate),
		scaleFixed_(scale) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Main() override;

	void Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) override;

	static void AfterDraw(rclmt::jobsys::Job* job, const int tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		rclmt::jobsys::run(link); }

	rclmt::jobsys::Job* DrawLower(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) {
		return rclmt::jobsys::make_job(DrawLowerJmp, std::tuple{this, dc, pmat, mvmat, link, depth}); }
	static void DrawLowerJmp(rclmt::jobsys::Job* job, const int tid, std::tuple<RepeatOp*, rglv::GL*, const rmlm::mat4* const, const rmlm::mat4* const, rclmt::jobsys::Job*, int>* data) {
		auto [self, dc, pmat, mvmat, link, depth] = *data;
		self->lowerNode_->Draw(dc, pmat, mvmat, link, depth); }

private:
	// config
	int cnt_;
	rmlv::vec3 translateFixed_{};
	rmlv::vec3 rotateFixed_{};
	rmlv::vec3 scaleFixed_{};

	// input
	GlNode *lowerNode_{nullptr};
	std::string scaleSlot_{"default"};
	ValuesBase* scaleNode_{nullptr};
	std::string rotateSlot_{"default"};
	ValuesBase* rotateNode_{nullptr};
	std::string translateSlot_{"default"};
	ValuesBase* translateNode_{nullptr}; };


class TranslateOp final : public GlNode {
public:
	TranslateOp(std::string_view id, InputList inputs)
		:GlNode(id, std::move(inputs)) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	void AddDeps() override;

	void Main() override;

	void Draw(rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) override;

private:
	GlNode *lowerNode_{nullptr};
	std::string scaleSlot_{"default"};
	ValuesBase* scaleNode_{nullptr};
	std::string rotateSlot_{"default"};
	ValuesBase* rotateNode_{nullptr};
	std::string translateSlot_{"default"};
	ValuesBase* translateNode_{nullptr}; };


}  // namespace rqv
}  // namespace rqdq
