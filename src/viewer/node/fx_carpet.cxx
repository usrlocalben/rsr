#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

namespace jobsys = rclmt::jobsys;

class Impl final : public IGl {
public:
	using IGl::IGl;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "freq") {
			freqNode_ = dynamic_cast<IValue*>(other);
			freqSlot_ = slot;
			if (freqNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "phase") {
			phaseNode_ = dynamic_cast<IValue*>(other);
			phaseSlot_ = slot;
			if (phaseNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot);}

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(freqNode_);
		AddDep(phaseNode_); }

	void Main() override {
		jobsys::run(Compute());}

	void Draw(int pass, rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, int depth [[maybe_unused]]) override {
		using namespace rglv;
		auto& dc = *_dc;
		if (pass != 1) return;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);
		// auto [id, ptr] = dc.AllocUniformBuffer<rglv::BaseProgram::UniformsSD>();
		// dc.UseUniforms(id);

		dc.UseBuffer(0, buffers_[activeBuffer_]);
		dc.DrawArrays(GL_TRIANGLES, 0, 6); }

public:
	jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		
			return jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }
private:
	static void ComputeJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl() {
		using rmlv::ivec2, rmlv::vec3, rmlv::vec2;
		activeBuffer_ = (activeBuffer_+1) % 3;
		auto& vao = buffers_[activeBuffer_];
		vao.clear();

		rmlv::vec3 freq{ 0.0F };
		rmlv::vec3 phase{ 0.0F };
		if (freqNode_ != nullptr) {
			freq = freqNode_->Eval(freqSlot_).as_vec3(); }
		if (phaseNode_ != nullptr) {
			phase = phaseNode_->Eval(phaseSlot_).as_vec3();}

		vec3 leftTopP{ -1.0F, 0.5F, 0 };
		vec3 leftTopT{ 0.0F, 1.0F, 0 };

		vec3 rightBottomP{ 1.0F, -0.5F, 0 };
		vec3 rightBottomT{ 1.0F, 0.0F, 0 };

		auto emitQuad = [&](vec3 ltp, vec3 ltt, vec3 rbp, vec3 rbt) {
			auto LT = ltp;                    auto RT = vec3{rbp.x, ltp.y, 0};
			auto LB = vec3{ltp.x, rbp.y, 0};  auto RB = rbp;
			{ // lower-left
				auto v0v2 = RB - LT;
				auto v0v1 = LB - LT;
				auto normal = normalize(cross(v0v1, v0v2));
				vao.append(LT, normal, vec3{ltt.x, ltt.y, 0});  // top left
				vao.append(LB, normal, vec3{ltt.x, rbt.y, 0});  // bottom left
				vao.append(RB, normal, vec3{rbt.x, rbt.y, 0});}  // bottom right
			{ // upper-right
				auto v0v2 = RT - LT;
				auto v0v1 = RB - LT;
				auto normal = normalize(cross(v0v1, v0v2));
				vao.append(LT, normal, vec3{ltt.x, ltt.y, 0});  // top left
				vao.append(RB, normal, vec3{rbt.x, rbt.y, 0});  // bottom right
				vao.append(RT, normal, vec3{rbt.x, ltt.y, 0});}};  // top right

		emitQuad(leftTopP, leftTopT, rightBottomP, rightBottomT);

		auto postSetup = jobsys::make_job(jobsys::noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}

private:
	// state
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	int activeBuffer_{0};

	// inputs
	IMaterial* materialNode_{nullptr};
	IValue* freqNode_{nullptr};
	std::string freqSlot_{};
	IValue* phaseNode_{nullptr};
	std::string phaseSlot_{}; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("freq", /*required=*/true)) { return; }
		if (!Input("phase", /*required=*/true)) { return; }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxCarpet", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
