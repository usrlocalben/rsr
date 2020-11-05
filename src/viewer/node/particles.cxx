#include <iostream>
#include <string_view>
#include <random>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlg/rmlg_noise.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_particles.hxx"
#include "src/viewer/compile.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace {

using namespace rqv;
using namespace rqdq::rmlm;
using namespace rqdq::rmlv;
namespace jobsys = rclmt::jobsys;

constexpr float oo = 9999999999.F;

struct Particle {
	rmlv::vec3 p;
	rmlv::vec3 v;
	float lr;
	float sz; };

class Impl final : public IGl, IParticles {

	double ttt_{0.0};
	const int many_;
	const float gravity_;
	const float life_;
	const float rate_;

	IGl* glNode_{nullptr};

	rcls::vector<float> px_, py_, pz_;
	rcls::vector<float> vx_, vy_, vz_;
	rcls::vector<float> lr_;
	rcls::vector<float> sz_;

	PixelToaster::Timer timer_;
	std::random_device rd_;
	std::minstd_rand rng_;
	std::uniform_real_distribution<float> newDist_{0.0F, 1.0F};

public:
	Impl(std::string_view id, InputList inputs, int many, float gravity, float life, float rate) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		IParticles(),
		many_(many),
		gravity_(gravity),
		life_(life),
		rate_(rate),
		px_(many_), py_(many_), pz_(many_),
		vx_(many_), vy_(many_), vz_(many_),
		lr_(many_), sz_(many_),
		rd_(),
		rng_(rd_()) {

		for (int i=0; i<many_; ++i) {
			px_[i] = oo; py_[i] = oo; pz_[i] = oo;
			vx_[i] = 0;  vy_[i] = 0;  vz_[i] = 0;
			lr_[i] = 0;
			sz_[i] = 1; }}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "gl") {
			glNode_ = dynamic_cast<IGl*>(other);
			if (glNode_ == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		IGl::AddDeps();
		AddDep(glNode_); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		glNode_ = nullptr; }

	void Main() override {
		auto updateJob = Update();
		AddLinksTo(updateJob);
		if (glNode_) {
			glNode_->AddLink(AfterAll(updateJob));
			glNode_->Run(); }
		else {
			jobsys::run(updateJob); }}

	// --- IGl ---
	void Draw(int pass, const LightPack& lights, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		if (!glNode_) return;
		if (pass != 1) return;
		for (int i=0; i<many_; ++i) {
			if (lr_[i] <= 0) continue;
			auto M = static_cast<rmlm::mat4*>(rclma::framepool::Allocate(64));
			*M = *mmat;
			*M = *M * mat4::translate(rmlv::vec3{ px_[i], py_[i], pz_[i] });
			glNode_->Draw(pass, lights, dc, pmat, vmat, M); }}

	// --- IParticles ---
	auto GetPtrs() const -> ParticlePtrs override {
		return { int(px_.size()), 
			     px_.data(), py_.data(), pz_.data(),
		         sz_.data() }; }

private:
	auto Update() -> jobsys::Job* {
		return jobsys::make_job(Impl::UpdateJmp, std::tuple{this}); }
	static void UpdateJmp(jobsys::Job*, unsigned, std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->UpdateImpl(); }
	void UpdateImpl() {
		const float dt = 1.0F/60.0F;
		ttt_ += dt;
		const auto t = ttt_; //timer_.time();
		for (int i=0; i<many_; ++i) {
			vy_[i] += gravity_;
			vx_[i] += (float)rmlg::PINoise(float(i)/many_, 0.1874, t) / 1000.0F;
			vz_[i] += (float)rmlg::PINoise(float(i)/many_, 0.5782, t*2) / 1000.0F;
			px_[i] += vx_[i]; py_[i] += vy_[i]; pz_[i] += vz_[i];
			lr_[i] -= dt;
			if (lr_[i] <= 0) {
				px_[i] = py_[i] = pz_[i] = oo;
				// vx_[i] = 0, vy_[i] = 0, vz_[i] = 0;
				if (newDist_(rng_) < rate_) {
					auto p = MakeOne();
					px_[i] = p.p.x; py_[i] = p.p.y; pz_[i] = p.p.z;
					vx_[i] = p.v.x; vy_[i] = p.v.y; vz_[i] = p.v.z;
					lr_[i] = p.lr;
					sz_[i] = p.sz; }}}}

	auto MakeOne() -> Particle {
		const float bounds = 0.5F;
		std::uniform_real_distribution<float> posDist(-bounds, bounds);
		std::uniform_real_distribution<float> dirDist(-2, 2);
		std::uniform_real_distribution<float> velDist(0.5, 1.0);
		std::uniform_real_distribution<float> sizDist(0.29, 0.31);

		// XXX this is not a uniform random vector
		const auto dir = normalize(rmlv::vec3{ dirDist(rng_), dirDist(rng_), dirDist(rng_) });
		const auto vel = dir * (velDist(rng_) / 60.0F);

		return {
			/*p=*/{ posDist(rng_), posDist(rng_), posDist(rng_) },
			/*v=*/vel,
			/*lr=*/life_,
			/*sz=*/sizDist(rng_) }; }
	};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		if (!Input("gl", /*required=*/false)) { return; }

		auto many = DataInt("many", 16);
		auto gravity = DataReal("gravity", 0.0F);
		auto life = DataReal("life", 1.0F);
		auto rate = DataReal("rate", 0.05F);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), many, gravity, life, rate); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$particles", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
}  // close enterprise namespace
