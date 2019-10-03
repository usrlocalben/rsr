#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string_view>
#include <utility>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

constexpr int many = 30000;


class Impl final : public IGl {
public:
	Impl(std::string_view id, InputList inputs, const rglv::Mesh& mesh) :
		IGl(id, std::move(inputs)) {
		using rmlm::mat4;
		using std::cout;

		/*
		cout << "==== BEGIN MESH DUMP ====\n";
		for (const auto& face : mesh.faces) {
			cout << "face\n";
			for (int i=0; i<3; ++i) {
				auto d0 = mesh.points[face.point_idx[i]];
				auto d1 = mesh.normals[face.normal_idx[i]];
				auto d2 = mesh.texcoords[face.texcoord_idx[i]];
				cout << i << ": " << d0 << " / " << d1 << " / " << d2 << "\n";
			}}
		cout << "====  END  MESH DUMP ====\n";
		*/

		auto [tmpVBO, tmpIdx] = make_indexed_vao_F3F3F3(mesh);
		vbo_ = tmpVBO;
		meshIndices_ = tmpIdx;

		float radius = 50.0F;
		float offset =  5.0F;

		std::random_device rd;
		std::mt19937 e2(rd());
		std::uniform_real_distribution<float> volDist(-radius, radius);
		std::uniform_real_distribution<float> rotDist(0, 6.28);

		mats_.resize(many);
		for (int i=0; i<many; ++i) {
			mat4& M = mats_[i];
			M = mat4::ident(); 

			float angle = i / float(many) * 3.1415926F * 2;
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

			M = M * mat4::rotate(rotDist(e2), normalize(rmlv::vec3{0.2F, 0.4F, 0.6F}));
			} }

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		/* if (attr == "num") {
			numNode_ = dynamic_cast<IValue*>(other);
			numSlot_ = slot;
			if (numNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; } */
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_); }
		// AddDep(numNode_); }

	void Main() override {
		rclmt::jobsys::run(Compute());}

	void Draw(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, rclmt::jobsys::Job* link, int depth) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		auto [id, ptr] = dc.AllocUniformBuffer<ManyProgram::UniformsSD>();
		ptr->pm = *pmat;
		ptr->mvm = *mvmat;
		ptr->nm = transpose(inverse(ptr->mvm));
		ptr->mvpm = ptr->pm * ptr->mvm;
		dc.UseUniforms(id);

		dc.UseBuffer(0, vbo_);
		dc.UseBuffer(1, (float*)mats_.data());
		dc.DrawElements(GL_TRIANGLES, meshIndices_.size(), GL_UNSIGNED_SHORT, meshIndices_.data(), many);
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }

public:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		return rclmt::jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }

private:
	static void ComputeJmp(rclmt::jobsys::Job* job, unsigned threadId, std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl() {
		using rmlv::ivec2;
		activeBuffer_ = (activeBuffer_+1)%3;

		/*rmlv::vec3 freq{ 0.0F };
		if (freqNode_ != nullptr) {
			freq = freqNode_->Eval(freqSlot_).as_vec3();

		rmlv::vec3 phase{ 0.0F };
		if (phaseNode_ != nullptr) {
			phase = phaseNode_->Eval(phaseSlot_).as_vec3(); }*/


		auto postSetup = rclmt::jobsys::make_job(rclmt::jobsys::noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}

private:
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
	};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		// if (!Input("freq", /*required=*/true)) { return; }
		// if (!Input("phase", /*required=*/true)) { return; }

		std::string_view meshPath{"notfound.obj"};
		if (auto jv = rclx::jv_find(data_, "mesh", JSON_STRING)) {
			meshPath = jv->toString(); }
		const auto& mesh = meshStore_->get(meshPath);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), mesh); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxMany", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
