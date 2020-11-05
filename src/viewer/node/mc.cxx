#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_marching_cubes.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders_envmap.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;


inline float sdSphere(const rmlv::vec3& pos, const float r) {
	return length(pos) - r; }


inline rmlv::qfloat sdSphere(const rmlv::qfloat3& pos, const float r) {
	return rmlv::length(pos) - r; }


struct AABB {
	rmlv::vec3 leftTopBack;
	rmlv::vec3 rightBottomFront; };


inline rmlv::vec3 midpoint(AABB block) {
	return mix(block.leftTopBack, block.rightBottomFront, 0.5F);}


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
		float distort = 0.60F * sin(5.0F*(pos.x + timeInSeconds_ / 4.0F))* sin(2.0F*(pos.y + (timeInSeconds_ / 1.33F))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0F) + (distort * sin(timeInSeconds_ / 2.0F) + 1.0F); }

	rmlv::mvec4f sample(rmlv::qfloat3 pos) const {
		using rmlv::mvec4f;
		auto T = mvec4f{ timeInSeconds_ };
		auto distort = mvec4f{0.60F} * sin(5.0F*(pos.x + T / 4.0F))* sin(2.0F*(pos.y + (T / 1.33F))); // *sin(50.0*sz);
		//return sdSphere(pos, 3.0f); }
		return sdSphere(pos, 3.0F) + (distort * sin(T / 2.0F) + 1.0F); }

	void Update(float t) {
		timeInSeconds_ = t; }

private:
	float timeInSeconds_; };


class Impl final : public IGl {
public:
	Impl(std::string_view id, InputList inputs, int precision, int forkDepth, float range) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		precision_(precision),
		forkDepth_(forkDepth),
		range_(range) {
		buffers_[0].reserve(4096);
		buffers_[1].reserve(4096);
		buffers_[2].reserve(4096); }

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "frob") {
			frobNode_ = dynamic_cast<IValue*>(other);
			frobSlot_ = slot;
			if (frobNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		materialNode_ = nullptr;
		frobNode_ = nullptr; }

	void Main() override {
		using rmlv::vec3;
		namespace jobsys = rclmt::jobsys;

		SwapBuffers();

		float frobValue = 0.0F;
		if (frobNode_ != nullptr) {
			frobValue = frobNode_->Eval(frobSlot_).as_float(); }
		field_.Update(frobValue);

		AABB rootBlock = {
			vec3{-range_, range_,-range_},
			vec3{ range_,-range_, range_} };

		const int subDim = precision_ >> forkDepth_;

		blockDivider_.Clear();
		blockDivider_.Compute(rootBlock, forkDepth_);

		auto finalizeJob = Finalize();
		std::array<jobsys::Job*, 4096> subJobs;  // enough for forkDepth==4
		int subJobEnd = 0;

		for (const auto& subBlock : blockDivider_.results_) {
			subJobs[subJobEnd++] = Resolve(subBlock, subDim, finalizeJob); }
		for (int sji = 0; sji < subJobEnd; sji++) {
			jobsys::run(subJobs[sji]); }
		jobsys::run(finalizeJob); }

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		auto& dc = *_dc;
		using rglv::GL_UNSIGNED_SHORT;
		using rglv::GL_CULL_FACE;
		using rglv::GL_TRIANGLES;
		std::lock_guard lock(dc.mutex);

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);

		auto& buffer = buffers_[activeBuffer_];
		for (int ai=0; ai<bufferEnd_[activeBuffer_]; ai++) {
			auto& vao = buffer[ai];
			if (vao.size() != 0) {
				const int elements = vao.size();
				vao.pad();
				dc.UseBuffer(0, vao.a0);
				dc.UseBuffer(3, vao.a1);
				dc.DrawArrays(GL_TRIANGLES, 0, elements); }}}

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		auto& dc = *_dc;
		using rglv::GL_UNSIGNED_SHORT;
		using rglv::GL_CULL_FACE;
		using rglv::GL_TRIANGLES;
		if (pass != 1) return;
		std::lock_guard lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);

		auto& buffer = buffers_[activeBuffer_];
		for (int ai=0; ai<bufferEnd_[activeBuffer_]; ai++) {
			auto& vao = buffer[ai];
			if (vao.size() != 0) {
				const int elements = vao.size();
				vao.pad();
				dc.UseBuffer(0, vao.a0);
				dc.UseBuffer(3, vao.a1);
				dc.DrawArrays(GL_TRIANGLES, 0, elements); }}}

private:
	void SwapBuffers() {
		activeBuffer_ = (activeBuffer_+1) % 3;
		bufferEnd_[activeBuffer_] = 0; }

	rglv::VertexArray_F3F3F3& AllocVAO() {
		std::scoped_lock lock(bufferMutex_);
		auto& buffer = buffers_[activeBuffer_];
		auto& end = bufferEnd_[activeBuffer_];
		if (end == int(buffer.size())) {
			if (buffer.capacity() == buffer.size()) {
				std::cerr << "buffer is full @ " << buffer.size() << std::endl;
				throw std::runtime_error("buffer full"); }
			buffer.push_back({}); }
		auto& vao = buffer[end++];
		vao.clear();
		return vao; }

	rclmt::jobsys::Job* Finalize() {
		return rclmt::jobsys::make_job(Impl::FinalizeJmp, std::tuple{this}); }
	static void FinalizeJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->FinalizeImpl(); }
	void FinalizeImpl() {
		RunLinks(); }

	rclmt::jobsys::Job* Resolve(AABB block, int dim, rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::ResolveJmp, std::tuple{this, block, dim}); }
		return rclmt::jobsys::make_job(Impl::ResolveJmp, std::tuple{this, block, dim}); }
	static void ResolveJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*, AABB, int>* data) {
		auto&[self, block, dim] = *data;
		self->ResolveImpl(block, dim);}
	void ResolveImpl(AABB block, int dim) {
		using rmlv::vec3, rmlv::mvec4f, rmlv::qfloat, rmlv::qfloat3;
		const int stride = 64;
		std::array<std::array<float, stride*stride>, 2> buf;
		int top = 1;
		int bot = 0;

		float sy = block.leftTopBack.y;
		float delta = (block.rightBottomFront.x - block.leftTopBack.x) / float(dim);
		const qfloat vdelta{ delta * 4 };

		const auto mid = midpoint(block);
		const auto R = length(block.leftTopBack - mid);
		float D = field_.sample(mid);
		if (abs(D)*0.5F > R) {
			return; }

		auto fillSlice = [&](){
			mvec4f fooZ{ block.leftTopBack.z };
			mvec4f fooY{ sy };
			for (int iz=0; iz<dim+1; iz++, fooZ += delta) {
				mvec4f fooX{ block.leftTopBack.x };
				fooX += mvec4f{ 0, delta, delta*2, delta*3 };
				for (int ix{0}; ix<dim+1; ix+=4, fooX+=vdelta) {
					auto distance = field_.sample({ fooX, fooY, fooZ });
					_mm_storeu_ps(&(buf[bot][iz*stride + ix]), distance.v); }}};

		fillSlice(); // load the first slice
		sy -= delta;
		auto& vao = AllocVAO();

		vec3 origin = block.leftTopBack;
		for (int iy=0; iy<dim; iy++, sy -= delta) {
			origin.y -= delta;
			std::swap(top, bot);
			fillSlice();

			origin.z = block.leftTopBack.z;
			for (int iz = 0; iz < dim; iz++, origin.z += delta) {
				origin.x = block.leftTopBack.x;
				for (int ix = 0; ix < dim; ix++, origin.x += delta) {
					rglv::Cell cell;
					cell.value[0] = buf[bot][ iz   *stride + ix];
					cell.pos[0] = origin;

					cell.value[1] = buf[bot][ iz   *stride + ix + 1];
					cell.pos[1] = vec3{ origin.x + delta, origin.y, origin.z };

					cell.value[2] = buf[top][ iz   *stride + ix + 1];
					cell.pos[2] = vec3{ origin.x + delta, origin.y + delta, origin.z };

					cell.value[3] = buf[top][ iz   *stride + ix];
					cell.pos[3] = vec3{ origin.x,         origin.y + delta, origin.z };

					cell.value[4] = buf[bot][(iz+1)*stride + ix];
					cell.pos[4] = vec3{ origin.x,         origin.y, origin.z + delta };

					cell.value[5] = buf[bot][(iz+1)*stride + ix + 1];
					cell.pos[5] = vec3{ origin.x + delta, origin.y, origin.z + delta };

					cell.value[6] = buf[top][(iz+1)*stride + ix + 1];
					cell.pos[6] = vec3{ origin.x + delta, origin.y + delta, origin.z + delta };

					cell.value[7] = buf[top][(iz+1)*stride + ix];
					cell.pos[7] = vec3{ origin.x,         origin.y + delta, origin.z + delta };
					rglv::march_sdf_vao(vao, delta, cell, field_); }}}}

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
	IMaterial* materialNode_{nullptr};
	IValue* frobNode_{nullptr};
	std::string frobSlot_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("frob", /*required=*/true)) { return; }

		auto precision = DataInt("precision", 32);
		switch (precision) {
		case 8:
		case 16:
		case 32:
		case 64:
		case 128:
			break;
		default:
			std::cerr << "FxMC: precision " << precision << " must be one of (8, 16, 32, 64, 128)!  precision will be 32\n";
			precision = 32; }

		auto forkDepth = DataInt("forkDepth", 2);
		if (forkDepth < 0 || forkDepth > 4) {
			std::cerr << "FxMC: forkDepth " << forkDepth << " exceeds limit (0 <= n <= 4)!  forkDepth will be 2\n";
			forkDepth = 2; }

		auto range = DataReal("range", 5.0F);
		if (range < 0.0001F) {
			std::cerr << "FxMC: range " << range << " is too small (n<0.0001)!  range will be +/- 5.0\n";
			range = 5.0F; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), precision, forkDepth, range); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$mc", [](){ return std::make_unique<Compiler>(); });
}} init{};

}  // namespace
}  // namespace rqdq
