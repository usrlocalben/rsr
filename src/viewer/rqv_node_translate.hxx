#pragma once
#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/rqv_node_gl.hxx"
#include "src/viewer/rqv_node_value.hxx"

#include <memory>
#include <string>
#include <tuple>

namespace rqdq {
namespace rqv {

const int DEPTH_FORK_UNTIL = 100;


struct RepeatOp : GlNode {
	// config
	int cnt;
	const rmlv::vec3 translate_fixed;
	const rmlv::vec3 rotate_fixed;
	const rmlv::vec3 scale_fixed;

	// input
	GlNode *lower = nullptr;
	std::string scale_source_slot = "default";
	ValuesBase* scale_source_node = nullptr;
	std::string rotate_source_slot = "default";
	ValuesBase* rotate_source_node = nullptr;
	std::string translate_source_slot = "default";
	ValuesBase* translate_source_node = nullptr;

	RepeatOp(
		const std::string& name,
		const InputList& inputs,
		const int cnt,
		const rmlv::vec3 translate,
		const rmlv::vec3 rotate,
		const rmlv::vec3 scale
	) :
		GlNode(name, inputs),
		cnt(cnt),
		translate_fixed(translate),
		rotate_fixed(rotate),
		scale_fixed(scale)
	{}

	void connect(const std::string& attr, NodeBase* node, const std::string& slot) override;
	std::vector<NodeBase*> deps() override;
	void main() override;

	void draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) override;

	static void after_draw(rclmt::jobsys::Job* job, const int tid, std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		rclmt::jobsys::run(link); }

	rclmt::jobsys::Job* drawLower(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job *link, int depth) {
		return rclmt::jobsys::make_job(drawLowerJmp, std::tuple{this, dc, pmat, mvmat, link, depth}); }
	static void drawLowerJmp(rclmt::jobsys::Job* job, const int tid, std::tuple<RepeatOp*, rglv::GL*, const rmlm::mat4* const, const rmlm::mat4* const, rclmt::jobsys::Job*, int>* data) {
		auto [self, dc, pmat, mvmat, link, depth] = *data;
		self->lower->draw(dc, pmat, mvmat, link, depth); }};


struct TranslateOp : GlNode {
	// input
	GlNode *lower = nullptr;
	std::string scale_source_slot = "default";
	ValuesBase* scale_source_node = nullptr;
	std::string rotate_source_slot = "default";
	ValuesBase* rotate_source_node = nullptr;
	std::string translate_source_slot = "default";
	ValuesBase* translate_source_node = nullptr;

	TranslateOp(const std::string& name, const InputList& inputs) :GlNode(name, inputs) {}

	void connect(const std::string& attr, NodeBase* other, const std::string& slot) override;
	std::vector<NodeBase*> deps() override;
	void main() override;

	void draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) override; };


}  // close package namespace
}  // close enterprise namespace
