#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <tuple>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_marching_cubes.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/gl.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

inline float sdSphere(const rmlv::vec3& pos, const float r) {
	return length(pos) - r; }

inline rmlv::qfloat sdSphere(const rmlv::qfloat3& pos, const float r) {
	return rmlv::length(pos) - r; }


}  // namespace

namespace rqv {

struct AABB {
	rmlv::vec3 left_top_back;
	rmlv::vec3 right_bottom_front; };


inline rmlv::vec3 midpoint(AABB block) {
	return mix(block.left_top_back, block.right_bottom_front, 0.5f);}


struct BlockDivider {
	std::vector<AABB> results;

	void compute(AABB _block, int limit) {
		using rmlv::vec3;
		if (limit == 0) {
			results.push_back(_block);
			return; }

		const auto mid = midpoint(_block);

		// BACK
		//   TOP
		//     LEFT
		AABB ltb{
			vec3{     _block.left_top_back.x,      _block.left_top_back.y, _block.left_top_back.z},
			vec3{                      mid.x,                       mid.y,                  mid.z} };
		//     RIGHT
		AABB rtb{
			vec3{                      mid.x,      _block.left_top_back.y, _block.left_top_back.z},
			vec3{_block.right_bottom_front.x,                       mid.y,                  mid.z} };
		//   BOTTOM
		//     LEFT
		AABB lbb{
			vec3{     _block.left_top_back.x,                       mid.y, _block.left_top_back.z},
			vec3{                      mid.x, _block.right_bottom_front.y,                  mid.z} };
		//     RIGHT
		AABB rbb{
			vec3{                      mid.x,                       mid.y, _block.left_top_back.z},
			vec3{_block.right_bottom_front.x, _block.right_bottom_front.y,                  mid.z} };
		// FRONT
		//   TOP
		//     LEFT
		AABB ltf{
			vec3{     _block.left_top_back.x,      _block.left_top_back.y,                       mid.z},
			vec3{                      mid.x,                       mid.y, _block.right_bottom_front.z} };
		//     RIGHT
		AABB rtf{
			vec3{                      mid.x,      _block.left_top_back.y,                       mid.z},
			vec3{_block.right_bottom_front.x,                       mid.y, _block.right_bottom_front.z} };
		//   BOTTOM
		//     LEFT
		AABB lbf{
			vec3{     _block.left_top_back.x,                       mid.y,                       mid.z},
			vec3{                      mid.x, _block.right_bottom_front.y, _block.right_bottom_front.z} };
		//     RIGHT
		AABB rbf{
			vec3{                      mid.x,                       mid.y,                       mid.z},
			vec3{_block.right_bottom_front.x, _block.right_bottom_front.y, _block.right_bottom_front.z} };

		std::array<AABB, 8> subBlocks = { ltb, rtb, lbb, rbb, ltf, rtf, lbf, rbf };
		for (const auto& block : subBlocks) {
			compute(block, limit - 1); }}

	void clear() {
		results.clear(); } };

struct Surface {
	float timeInSeconds;

	float sample(rmlv::vec3 pos) const {
		float distort = 0.60f * sin(5.0f*(pos.x + timeInSeconds / 4.0f))* sin(2.0f*(pos.y + (timeInSeconds / 1.33f))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0f) + (distort * sin(timeInSeconds / 2.0f) + 1.0f); }

	rmlv::mvec4f sample(rmlv::qfloat3 pos) const {
		using rmlv::mvec4f;
		auto T = mvec4f{ timeInSeconds };
		auto distort = mvec4f{0.30f} * sin(5.0f*(pos.x + T / 4.0f))* sin(2.0f*(pos.y + (T / 1.33f))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0f) + (distort * sin(T / 2.0f) + 1.0f); }

	void update(float t) {
		timeInSeconds = t; }};


struct FxMC : GlNode {
	Surface d_field;

	std::array<std::vector<rglv::VertexArray_F3F3F3>, 3> d_buffers;
	std::array<std::atomic<int>, 3> d_bufferEnd = { 0,0,0 };
	std::atomic<int> d_activeBuffer = 0;
	std::mutex d_bufferMutex;

	BlockDivider blockDivider;

	// config
	rglr::Texture d_envmap;
	const int d_precision;
	const int d_forkDepth;
	ShaderProgramId d_program;
	const float d_range;

	// connections
	MaterialNode* material_node = nullptr;
	ValuesBase* frob_node = nullptr;
	std::string frob_slot;

	FxMC(
		const std::string& name,
		const InputList& inputs,
		const int precision,
		const int forkDepth,
		const float range
	) :GlNode(name, inputs), d_precision(precision), d_forkDepth(forkDepth), d_range(range) {
		d_buffers[0].reserve(4096);
		d_buffers[1].reserve(4096);
		d_buffers[2].reserve(4096); }

	void connect(const std::string& /*attr*/, NodeBase* /*other*/, const std::string& /*slot*/) override;
	void main() override;

	void draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override;

	void swapBuffers();
	rglv::VertexArray_F3F3F3& allocVAO();

	rclmt::jobsys::Job* finalize() {
		return rclmt::jobsys::make_job(FxMC::finalizeJmp, std::tuple{this}); }
	static void finalizeJmp(rclmt::jobsys::Job* jobptr, unsigned threadId, std::tuple<FxMC*>* data) {
		auto&[self] = *data;
		self->finalizeImpl(); }
	void finalizeImpl() {
		// XXX could possibly defer to nodebase::main();
		for (auto link : links) {
			rclmt::jobsys::run(link); }}

	rclmt::jobsys::Job* resolve(AABB block, int dim, rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, FxMC::resolveJmp, std::tuple{this, block, dim}); }
		return rclmt::jobsys::make_job(FxMC::resolveJmp, std::tuple{this, block, dim}); }
	static void resolveJmp(rclmt::jobsys::Job* jobptr, unsigned threadId, std::tuple<FxMC*, AABB, int>* data) {
		auto&[self, block, dim] = *data;
		self->resolveImpl(block, dim, threadId);}
	void resolveImpl(AABB block, int dim, int threadId); };


}  // namespace rqv
}  // namespace rqdq
