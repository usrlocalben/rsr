#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_marching_cubes.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_particles.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

template<int EXP>
constexpr auto pow(int a) -> int {
	if constexpr (EXP == 0) {
		return 1; }
	else {
		return a * pow<EXP-1>(a); }}

constexpr auto pow(int a, int ex) -> int {
	if (ex == 0) return 1;
	if (ex == 1) return a;
	int tmp = pow(a, ex/2);
	if (ex%2 == 0) {
		return tmp*tmp; }
	else {
		return a*tmp*tmp; }}

inline
int FindAndClearLSB(uint32_t& x) {
	unsigned long idx;
	_BitScanForward(&idx, x);
	x &= x - 1;
	return idx; }

inline
auto sd_sphere(const rmlv::qfloat3& p, const rmlv::qfloat& r) -> rmlv::qfloat {
	return length(p) - r; }

inline
auto Charge(const rmlv::qfloat3& p, const rmlv::qfloat& r) -> rmlv::qfloat {
	auto d = length(p);
	
	auto inside = cmpgt(r, d);

	auto left = (2 * d*d*d) / (r*r*r);
	auto right = (3 * d*d) / (r*r);
	auto charge = left - right + 1;
	return SelectFloat(_mm_setzero_ps(), charge, inside); }


class Impl : public IGl {
	// config
	const int dim_;
	const int half_;
	const int levels_;
	const float scale_;

	IParticles* particlesNode_{nullptr};
	IMaterial* materialNode_{nullptr};

	int mod2_{0};
	std::mutex bufferMutex_{};
	std::array<std::vector<rglv::VertexArray_F3F3F3>, 2> buffers_;
	std::array<std::atomic<int>, 2> bufferEnd_{ 0, 0 };

	std::array<rcls::vector<float>, 600> bpx_, bpy_, bpz_, bsz_;
	std::atomic<int> packsEnd_{0};

	std::atomic<int> blocksRemaining_{};
	rcls::vector<float> cloud_;
	ParticlePtrs pptrs_;

public:
	Impl(std::string_view id, InputList inputs, int dim, int levels, float scale) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		dim_(dim),
		half_(dim_/2),
		levels_(levels),
		scale_(scale) {
		assert(dim_==16 || dim_==32 || dim_==64 || dim_==128);
		assert(0 <= levels_ && levels_ <= 2);
		cloud_.resize(pow<3>(dim_)); }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "particles") {
			particlesNode_ = dynamic_cast<IParticles*>(other);
			if (particlesNode_ == nullptr) {
				TYPE_ERROR(IParticles);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(particlesNode_); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		particlesNode_ = nullptr; }

	void Main() override {
		auto updateJob = Update();
		if (particlesNode_) {
			particlesNode_->AddLink(AfterAll(updateJob));
			particlesNode_->Run(); }
		else {
			jobsys::run(updateJob); }}

	// --- IGl ---
	void Draw(int pass, const LightPack& lights, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {}

private:
	struct BlockRange {
		int16_t x1, x2;
		int16_t y1, y2;
		int16_t z1, z2;
		int16_t l; };

	auto Update() -> jobsys::Job* {
		return jobsys::make_job(Impl::UpdateJmp, std::tuple{this}); }
	static void UpdateJmp(jobsys::Job*, unsigned, std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->UpdateImpl(); }
	void UpdateImpl() {
		mod2_ = (mod2_+1)&1;
		bufferEnd_[mod2_] = 0;
		packsEnd_ = 0;

		blocksRemaining_ = pow(8, levels_);

		pptrs_ = particlesNode_->GetPtrs();
		jobsys::run(Block(RootRange())); }

	auto Block(BlockRange range, jobsys::Job* parent=nullptr) -> jobsys::Job* {
		if (parent != nullptr) {
			return jobsys::make_job_as_child(parent, Impl::BlockJmp, std::tuple{this, range}); }
		return jobsys::make_job(Impl::BlockJmp, std::tuple{this, range}); }
	static void BlockJmp(jobsys::Job*, unsigned, std::tuple<Impl*, BlockRange>* data) {
		auto&[self, range] = *data;
		self->BlockImpl(range);}
	void BlockImpl(BlockRange range) {
		if (range.l == 0) {
			Fill(range);
			if (--blocksRemaining_ == 0) {
				// std::cerr << "done resolving cloud\n";
				jobsys::run(Resolve(RootRange())); }}
		else {
			int16_t lm1 = range.l - 1;
			int16_t x1 = range.x1, x2 = range.x2;
			int16_t y1 = range.y1, y2 = range.y2;
			int16_t z1 = range.z1, z2 = range.z2;
			int16_t xm = (x1 + x2)/2;
			int16_t ym = (y1 + y2)/2;
			int16_t zm = (z1 + z2)/2;
			jobsys::run(Block({ x1, xm, y1, ym, z1, zm, lm1 }));
			jobsys::run(Block({ xm, x2, y1, ym, z1, zm, lm1 }));
			jobsys::run(Block({ x1, xm, ym, y2, z1, zm, lm1 }));
			jobsys::run(Block({ xm, x2, ym, y2, z1, zm, lm1 }));
			jobsys::run(Block({ x1, xm, y1, ym, zm, z2, lm1 }));
			jobsys::run(Block({ xm, x2, y1, ym, zm, z2, lm1 }));
			jobsys::run(Block({ x1, xm, ym, y2, zm, z2, lm1 }));
			jobsys::run(Block({ xm, x2, ym, y2, zm, z2, lm1 })); }}

	void Fill(BlockRange r) {
		using rmlv::ivec3, rmlv::vec3, rmlv::qfloat, rmlv::qfloat3, rmlv::mvec4i;

		thread_local rcls::vector<float> mx, my, mz, mr;
		mx.clear(); my.clear(); mz.clear(); mr.clear();

		ivec3 ic1(r.x1, r.y1, r.z1);  // - corner (inclusive)
		ivec3 ic2(r.x2, r.y2, r.z2);  // + corner (exclusive!)

		vec3 c1 = itof(ic1 - half_) / float(dim_);
		vec3 c2 = itof(ic2 - half_) / float(dim_);

		// oct-tree breakdown of pow-2 cube should
		// give SSE friendly dimensions at each level
		assert((r.x2 - r.x1) % 4 == 0);

		const auto boxMin = qfloat3{ c1 };
		const auto boxMax = qfloat3{ c2 };

		const int pcnt = pptrs_.many;
		for (int bi=0; bi<pcnt; bi+=4) {

			qfloat3 ballPos{ _mm_load_ps(&pptrs_.px[bi]),
			                 _mm_load_ps(&pptrs_.py[bi]),
			                 _mm_load_ps(&pptrs_.pz[bi]) };

			qfloat ballRadius{ _mm_load_ps(&pptrs_.sz[bi]) };
			auto ballRadiusSq = ballRadius * ballRadius;

			auto nearestPoint = clamp(ballPos, boxMin, boxMax);
			auto ballToBox = ballPos - nearestPoint;
			auto distanceSq = dot(ballToBox, ballToBox);
			auto intersects = cmplt(distanceSq, ballRadiusSq);

			// auto active = rmlv::cmpgt(rmlv::qfloat{_mm_load_ps(&pptrs_.lr[bi])}, rmlv::qfloat{_mm_setzero_ps()});
			uint32_t valid = movemask(intersects); // & active);
			while (valid) {
				int li = FindAndClearLSB(valid);
				mx.push_back(pptrs_.px[bi+li]);
				my.push_back(pptrs_.py[bi+li]);
				mz.push_back(pptrs_.pz[bi+li]);
				mr.push_back(pptrs_.sz[bi+li]); }}


		const mvec4i xgrad{ 0, 1, 2, 3 };
		for (int y=r.y1; y<r.y2; ++y) {
			const qfloat py{ToReal(y)};
			for (int z=r.z1; z<r.z2; ++z) {
				const qfloat pz{ToReal(z)};
				float* out = DstRow(y, z);
				for (int x=r.x1; x<r.x2; x+=4) {
					const auto px = ToReal(mvec4i{x}+xgrad);

					// sum charges
					qfloat charge{ _mm_setzero_ps() };
					for (int bi=0, siz=int(mx.size()); bi<siz; ++bi) {
						qfloat3 ballPos{ mx[bi], my[bi], mz[bi] };
						qfloat ballRadius{ mr[bi] };
						charge += Charge(qfloat3{ px, py, pz } - ballPos, ballRadius); }

					_mm_stream_ps(&out[x], charge.v); }}} }

	auto Resolve(BlockRange range, jobsys::Job* parent=nullptr) -> jobsys::Job* {
		if (parent != nullptr) {
			return jobsys::make_job_as_child(parent, Impl::ResolveJmp, std::tuple{this, range}); }
		return jobsys::make_job(Impl::ResolveJmp, std::tuple{this, range}); }
	static void ResolveJmp(jobsys::Job*, unsigned, std::tuple<Impl*, BlockRange>* data) {
		auto&[self, range] = *data;
		self->ResolveImpl(range);}
	void ResolveImpl(BlockRange range) {
		RunLinks(); }

	auto RootRange() const -> BlockRange {
		return {0, int16_t(dim_),
		        0, int16_t(dim_),
		        0, int16_t(dim_),
		        int16_t(levels_)}; }

	/*
	auto AllocPack() -> int {
		return packsEnd_++; }
		std::scoped_lock lock(bufferMutex_);
		if (packsEnd_ == int(bpx_.size())) {
			bpx_.push_back({});
			bpy_.push_back({});
			bpz_.push_back({});
			bsz_.push_back({}); }
		++packsEnd_;
		return out; }
	*/

	/*
	auto AllocBuffer() -> rglv::VertexArray_F3F3F3& {
		std::scoped_lock lock(bufferMutex_);
		auto& buffer = buffers_[mod2_];
		auto& end = bufferEnd_[mod2_];
		if (end == int(buffer.size())) {
			if (buffer.capacity() == buffer.size()) {
				std::cerr << "buffer is full @ " << buffer.size() << std::endl;
				throw std::runtime_error("buffer full"); }
			buffer.push_back({}); }
		auto& vbo = buffer[end++];
		vbo.clear();
		return vbo; }
	*/

	auto ToReal(int i) -> float {
		return (i-half_)/float(dim_)*scale_; }

	auto ToReal(rmlv::mvec4i i) -> rmlv::qfloat {
		return itof(i-rmlv::mvec4i{half_}) / float(dim_) * scale_; }

	auto DstRow(int y, int z) -> float* {
		return &cloud_[y*dim_*dim_ + z*dim_]; }

	};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("material", /*required=*/false)) { return; }
		if (!Input("particles", /*required=*/true)) { return; }

		int precision{32};
		if (auto jv = jv_find(data_, "precision", JSON_NUMBER)) {
			precision = static_cast<int>(jv->toNumber()); }
		switch (precision) {
		case 16:
		case 32:
		case 64:
		case 128:
			break;
		default:
			std::cerr << "FxMC: precision " << precision << " must be one of (16, 32, 64, 128)!  precision will be 32\n";
			precision = 32; }

		int forkDepth{2};
		if (auto jv = jv_find(data_, "forkDepth", JSON_NUMBER)) {
			forkDepth = static_cast<int>(jv->toNumber()); }
		if (forkDepth < 0 || forkDepth > 4) {
			std::cerr << "FxMC: forkDepth " << forkDepth << " exceeds limit (0 <= n <= 4)!  forkDepth will be 2\n";
			forkDepth = 2; }

		float scale{1.0F};
		if (auto jv = jv_find(data_, "scale", JSON_NUMBER)) {
			scale = static_cast<float>(jv->toNumber()); }
		if (scale < 0.0001F) {
			std::cerr << "FxMC: scale " << scale << " is too small (n<0.0001)!  scale will be 1.0\n";
			scale = 1.0F; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), precision, forkDepth, scale); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$mc2", [](){ return std::make_unique<Compiler>(); });
}} init{};

}  // namespace
}  // namespace rqdq

/*

Particles{ id="particles",
  many=100, bounds=,  }

MetaBallField{ id="field",
  dim=67, balls="particles" }

MarchingCubes{ id="mc",
  field="field", dim=64, origin=1,1,1 }

*/


/*

dim=4

|x|x|x|x
0 1 2 3

4/2=2
0-2 = -2,   -1, 0, 1
 /2 = -1.0 -0.5 0 0.5

dim=8

|x|x|x|x|x|x|x|x
0 1 2 3 4 5 6 7 

8/2 = 4

   -4 -3 -2 -1  0  1  2  3
/4 -1.0
   */
