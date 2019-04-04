#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
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
	rmlv::vec3 leftTopBack;
	rmlv::vec3 rightBottomFront; };


inline rmlv::vec3 midpoint(AABB block) {
	return mix(block.leftTopBack, block.rightBottomFront, 0.5f);}


class BlockDivider {
public:
	std::vector<AABB> results_;

	void Compute(AABB _block, int limit) {
		using rmlv::vec3;
		if (limit == 0) {
			results_.emplace_back(_block);
			return; }

		const auto mid = midpoint(_block);
		const auto ltb = _block.leftTopBack;
		const auto rbf = _block.rightBottomFront;

		const auto subBlocks = std::array{
			// BACK
			//   TOP
			//     LEFT
			AABB{ vec3{ ltb.x, ltb.y, ltb.z }, vec3{ mid.x, mid.y, mid.z } },
			//     RIGHT
			AABB{ vec3{ mid.x, ltb.y, ltb.z }, vec3{ rbf.x, mid.y, mid.z } },
			//   BOTTOM
			//     LEFT
			AABB{ vec3{ ltb.x, mid.y, ltb.z }, vec3{ mid.x, rbf.y, mid.z } },
			//     RIGHT
			AABB{ vec3{ mid.x, mid.y, ltb.z }, vec3{ rbf.x, rbf.y, mid.z } },
			// FRONT
			//   TOP
			//     LEFT
			AABB{ vec3{ ltb.x, ltb.y, mid.z }, vec3{ mid.x, mid.y, rbf.z } },
			//     RIGHT
			AABB{ vec3{ mid.x, ltb.y, mid.z }, vec3{ rbf.x, mid.y, rbf.z } },
			//   BOTTOM
			//     LEFT
			AABB{ vec3{ ltb.x, mid.y, mid.z }, vec3{ mid.x, rbf.y, rbf.z } },
			//     RIGHT
			AABB{ vec3{ mid.x, mid.y, mid.z }, vec3{ rbf.x, rbf.y, rbf.z } } };

		for (const auto& subBlock : subBlocks) {
			Compute(subBlock, limit - 1); }}

	void Clear() {
		results_.clear(); } };


struct Surface {
	float sample(rmlv::vec3 pos) const {
		float distort = 0.60f * sin(5.0f*(pos.x + timeInSeconds_ / 4.0f))* sin(2.0f*(pos.y + (timeInSeconds_ / 1.33f))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0f) + (distort * sin(timeInSeconds_ / 2.0f) + 1.0f); }

	rmlv::mvec4f sample(rmlv::qfloat3 pos) const {
		using rmlv::mvec4f;
		auto T = mvec4f{ timeInSeconds_ };
		auto distort = mvec4f{0.30f} * sin(5.0f*(pos.x + T / 4.0f))* sin(2.0f*(pos.y + (T / 1.33f))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0f) + (distort * sin(T / 2.0f) + 1.0f); }

	void Update(float t) {
		timeInSeconds_ = t; }

private:
	float timeInSeconds_; };


class FxMC final : public GlNode {
public:
	FxMC(std::string_view id, InputList inputs, int precision, int forkDepth, float range)
		:GlNode(id, std::move(inputs)), precision_(precision), forkDepth_(forkDepth), range_(range) {
		buffers_[0].reserve(4096);
		buffers_[1].reserve(4096);
		buffers_[2].reserve(4096); }

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;
	void Main() override;

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override;

private:
	void SwapBuffers();
	rglv::VertexArray_F3F3F3& AllocVAO();

	rclmt::jobsys::Job* Finalize() {
		return rclmt::jobsys::make_job(FxMC::FinalizeJmp, std::tuple{this}); }
	static void FinalizeJmp(rclmt::jobsys::Job* jobptr, unsigned threadId, std::tuple<FxMC*>* data) {
		auto&[self] = *data;
		self->FinalizeImpl(); }
	void FinalizeImpl() {
		RunLinks(); }

	rclmt::jobsys::Job* Resolve(AABB block, int dim, rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, FxMC::ResolveJmp, std::tuple{this, block, dim}); }
		return rclmt::jobsys::make_job(FxMC::ResolveJmp, std::tuple{this, block, dim}); }
	static void ResolveJmp(rclmt::jobsys::Job* jobptr, unsigned threadId, std::tuple<FxMC*, AABB, int>* data) {
		auto&[self, block, dim] = *data;
		self->ResolveImpl(block, dim, threadId);}
	void ResolveImpl(AABB block, int dim, int threadId);

private:
	Surface field_;

	std::array<std::vector<rglv::VertexArray_F3F3F3>, 3> buffers_;
	std::array<std::atomic<int>, 3> bufferEnd_{ 0, 0, 0 };
	std::atomic<int> activeBuffer_{0};
	std::mutex bufferMutex_{};

	BlockDivider blockDivider_{};

	// config
	rglr::Texture envmap_;
	int precision_;
	int forkDepth_;
	float range_;

	// connections
	MaterialNode* materialNode_{nullptr};
	ValuesBase* frobNode_{nullptr};
	std::string frobSlot_; };


}  // namespace rqv
}  // namespace rqdq
