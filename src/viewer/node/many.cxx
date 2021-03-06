#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_mesh_util.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string_view>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

constexpr int batch = 3000;

class Impl final : public IGl {

	// state
	int activeBuffer_{0};
	rglv::VertexArray_F3F3F3 vbo_;
	rcls::vector<uint16_t> meshIndices_;
	rcls::vector<rmlm::mat4> mats_;
	// std::array<rglv::VertexArray_F3F3F3, 3> buffers_{};

	// inputs
	IMaterial* materialNode_{nullptr};
	/*IValue* freqNode_{nullptr};
	std::string freqSlot_{};
	IValue* phaseNode_{nullptr};
	std::string phaseSlot_{};*/

public:
	Impl(std::string_view id, InputList inputs, const rglv::Mesh& mesh) :
		NodeBase(id, std::move(inputs)),
		IGl() {
		using rmlm::mat4;
		using std::cout;

		rglv::MakeArray(mesh, "PNT", vbo_, meshIndices_);

		float radius = 50.0F;
		float offset =  5.0F;

		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<float> volDist(-radius, radius);
		std::uniform_real_distribution<float> rotDist(0, 6.28);

		mats_.reserve(batch*3);

		radius = 20.0F;
		offset = 2.5F;
		for (int i=0; i<batch; ++i) {
			mat4& M = mats_.emplace_back();
			M = mat4{1.0F};

			float angle = i / float(batch) * 3.1415926F * 2;
			float displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float x = sin(angle) * radius + displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float y = displacement * 0.4F;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float z = cos(angle) * radius + displacement;

			M = M * mat4::translate({ x, y, z });

			float scale = std::uniform_real_distribution<float>(0.10F, 0.30F)(e2);
			M = M * mat4::scale(rmlv::vec3{scale});

			M = M * mat4::rotate(rotDist(e2), normalize(rmlv::vec3{0.2F, 0.4F, 0.6F})); }

		radius = 40.0F;
		offset = 4.5F;
		for (int i=0; i<batch; ++i) {
			mat4& M = mats_.emplace_back();
			M = mat4{1.0F};

			float angle = i / float(batch) * 3.1415926F * 2;
			float displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float x = sin(angle) * radius + displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float y = displacement * 0.4F;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float z = cos(angle) * radius + displacement;

			M = M * mat4::translate({ x, y, z });

			float scale = std::uniform_real_distribution<float>(0.10F, 0.30F)(e2);
			M = M * mat4::scale(rmlv::vec3{scale});

			M = M * mat4::rotate(rotDist(e2), normalize(rmlv::vec3{0.2F, 0.4F, 0.6F})); }

		offset = 50.0F;
		offset = 10.0F;
		for (int i=0; i<batch; ++i) {
			mat4& M = mats_.emplace_back();
			M = mat4{1.0F};

			float angle = i / float(batch) * 3.1415926F * 2;
			float displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float x = sin(angle) * radius + displacement;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float y = displacement * 0.4F;
			displacement = std::uniform_real_distribution<float>(-offset, offset)(e2);
			float z = cos(angle) * radius + displacement;

			M = M * mat4::translate({ x, y, z });

			float scale = std::uniform_real_distribution<float>(0.10F, 0.30F)(e2);
			M = M * mat4::scale(rmlv::vec3{scale});

			M = M * mat4::rotate(rotDist(e2), normalize(rmlv::vec3{0.2F, 0.4F, 0.6F})); } }

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void DisconnectAll() override {
		IGl::DisconnectAll();
		materialNode_ = nullptr; }

	void AddDeps() override {
		AddDep(materialNode_); }

	void Main() override {
		jobsys::run(Compute());}

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard lock(dc.mutex);

		dc.ProjectionMatrix(*pmat);
		dc.ViewMatrix(*mvmat);
		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.222F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+0));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);}

		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.555F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+batch));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);}

		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.888F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+batch+batch));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);} }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		if (pass != 1) return;
		std::lock_guard lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ProjectionMatrix(*pmat);
		dc.ViewMatrix(*vmat * *mmat);
		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.222F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+0));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);}

		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.555F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+batch));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);}

		{auto [id, ptr] = dc.AllocUniformBuffer();
		static_cast<ManyProgram::UniformsSD*>(ptr)->magic = 0.888F;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_.a0);  // P
		dc.UseBuffer(3, vbo_.a1);  // N
		dc.UseBuffer(9, vbo_.a2.x.data());  // T.u
		dc.UseBuffer(10, vbo_.a2.y.data()); // T.v
		dc.UseBuffer(15, (float*)(mats_.data()+batch+batch));
		dc.DrawElementsInstanced(GL_TRIANGLES, static_cast<int>(meshIndices_.size()), GL_UNSIGNED_SHORT, meshIndices_.data(), batch);}
		}

public:
	auto Compute(jobsys::Job* parent = nullptr) -> jobsys::Job* {
		if (parent != nullptr) {
			return jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		return jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }

private:
	static void ComputeJmp(jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl() {
		using rmlv::ivec2;
		activeBuffer_ = (activeBuffer_+1)%3;

		auto postSetup = jobsys::make_job(jobsys::noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();} };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }

		auto meshPath = DataString("mesh", "notfound.obj");
		const auto& mesh = meshStore_->get(meshPath);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), mesh); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$many", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
