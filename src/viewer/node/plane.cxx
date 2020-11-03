#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"
#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl : public IGl {
	rglv::VertexArray_F3F3F3 vbo_;

	// connections
	IMaterial* materialNode_{nullptr};

public:
	Impl(std::string_view id, InputList inputs) :
		NodeBase(id, std::move(inputs)),
		IGl() {
		using rmlv::vec3;

		rmlv::vec2 leftTop{ -0.5F, 0.5F };
		rmlv::vec2 rightBottom{ 0.5F, -0.5F };
		float z = 0.0F;

		vec3 pul{ leftTop.x, leftTop.y, z };     vec3 pur{ rightBottom.x, leftTop.y, z };
		vec3 tul{ 0.0F, 1.0F, 0 };               vec3 tur{ 1.0F, 1.0F, 0 };

		vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
		vec3 tll{ 0.0F, 0.0F, 0 };               vec3 tlr{ 1.0F, 0.0F, 0 };

		vec3 normal{ 0, 0, 1.0F };

		// quad upper left
		vbo_.append(pur, normal, tur);
		vbo_.append(pul, normal, tul);
		vbo_.append(pll, normal, tll);

		// quad lower right
		vbo_.append(pur, normal, tur);
		vbo_.append(pll, normal, tll);
		vbo_.append(plr, normal, tlr);
		vbo_.pad(); }

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
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
		using namespace rclmt::jobsys;
		using rmlv::ivec2, rmlv::vec3;

		Job* postSetup = make_job(noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat) override {
		auto& dc = *_dc;
		using namespace rglv;
		std::scoped_lock lock(dc.mutex);

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, vbo_.a0);
		dc.UseBuffer(3, vbo_.a1);
		dc.UseBuffer(9, vbo_.a2);
		dc.DrawArrays(GL_TRIANGLES, 0, 6); }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const vmat, const rmlm::mat4* mmat) override {
		auto& dc = *_dc;
		using namespace rglv;
		if (pass != 1) return;

		std::scoped_lock lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);
		dc.UseBuffer(0, vbo_.a0);
		dc.UseBuffer(3, vbo_.a1);
		dc.UseBuffer(9, vbo_.a2);
		dc.DrawArrays(GL_TRIANGLES, 0, 6); }};


class DepthDebug final : public IGl {
	rglv::VertexArray_F3F3F3 vbo_;

public:
	DepthDebug(std::string_view id, InputList inputs) :
		NodeBase(id, std::move(inputs)),
		IGl() {
		using rmlv::vec3;

		rmlv::vec2 leftTop{ -0.5F, 0.5F };
		rmlv::vec2 rightBottom{ 0.5F, -0.5F };
		float z = 0.0F;

		vec3 pul{ leftTop.x, leftTop.y, z };     vec3 pur{ rightBottom.x, leftTop.y, z };
		vec3 tul{ 0.0F, 1.0F, 0 };               vec3 tur{ 1.0F, 1.0F, 0 };

		vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
		vec3 tll{ 0.0F, 0.0F, 0 };               vec3 tlr{ 1.0F, 0.0F, 0 };

		vec3 normal{ 0, 0, 1.0F };

		// quad upper left
		vbo_.append(pur, normal, tur);
		vbo_.append(pul, normal, tul);
		vbo_.append(pll, normal, tll);

		// quad lower right
		vbo_.append(pur, normal, tur);
		vbo_.append(pll, normal, tll);
		vbo_.append(plr, normal, tlr);
		vbo_.pad(); }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* const pmat [[maybe_unused]], const rmlm::mat4* const vmat [[maybe_unused]], const rmlm::mat4* mmat [[maybe_unused]]) override {
		auto& dc = *_dc;
		using namespace rglv;
		if (pass != 1) return;
		std::scoped_lock lock(dc.mutex);

		auto pm = rglv::Orthographic(0, 8, 0, 4.5, 1.0F, -1.0F);
		dc.ProjectionMatrix(pm);
		auto vm = rmlm::mat4::translate(1, 1, 0);
		dc.ViewMatrix(vm);
		dc.UseProgram(5);  // DepthProgram
		dc.BindTexture3(lights.map[0], lights.size[0]);
		dc.UseBuffer(0, vbo_.a0);
		dc.UseBuffer(3, vbo_.a1);
		dc.UseBuffer(9, vbo_.a2);
		dc.DrawArrays(GL_TRIANGLES, 0, 6); }};


class SpotLight final : public IGl {

	// config
	const float angle_;
	const int size_;

public:
	SpotLight(std::string_view id, InputList inputs, float angle, int size) :
		NodeBase(id, std::move(inputs)),
		IGl(),
		angle_(angle),
		size_(size) {}

	auto Lights(rmlm::mat4 mvmat) -> LightPack override {
		// mvmat is lightToWorld
		LightPack out;
		out.cnt = 1;
		out.vmat[0] = mvmat;  // the caller will convert vmat into a viewmatrix
		out.angle[0] = angle_;
		out.size[0] = size_;
		return out; }};


class SpotLightCompiler final : public NodeCompiler {
	void Build() override {
		using namespace rclx;
		float angle{90};
		if (auto jv = jv_find(data_, "angle", JSON_NUMBER)) {
			angle = static_cast<float>(jv->toNumber()); }
		int size{256};
		if (auto jv = jv_find(data_, "size", JSON_NUMBER)) {
			auto n = static_cast<int>(jv->toNumber());
			if (n == 1024 || n == 512 || n == 256 || n == 128 || n == 64 || n == 32 || n == 16) {
				size = n; }
			else {
				std::cerr << "bad spot map size " << n << ", will use 256\n"; }}
		out_ = std::make_shared<SpotLight>(id_, std::move(inputs_), angle, size); }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


class DepthDebugCompiler final : public NodeCompiler {
	void Build() override {
		out_ = std::make_shared<DepthDebug>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$plane", [](){ return std::make_unique<Compiler>(); });
	NodeRegistry::GetInstance().Register("$spot", [](){ return std::make_unique<SpotLightCompiler>(); });
	NodeRegistry::GetInstance().Register("$depthDebug", [](){ return std::make_unique<DepthDebugCompiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
