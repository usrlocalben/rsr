#include <array>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/shaders.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl final : public IGl {
	int activeBuffer_{0};
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	rcls::vector<uint16_t> meshIdx_;

	// connections
	IMaterial* materialNode_{nullptr};
	/*IValue* leftTopNode_{nullptr};
	std::string leftTopSlot_{};
	IValue* rightBottomNode_{nullptr};
	std::string rightBottomSlot_{};
	IValue* zNode_{nullptr};
	std::string zSlot_{};*/

	// config
	// rmlv::vec2 leftTop_;
	// rmlv::vec2 rightBottom_;
	// float z_;

public:
	using IGl::IGl;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_); }

	void Reset() override {
		activeBuffer_ = (activeBuffer_+1)%3;
		IGl::Reset(); }

	void Main() override {
		using namespace rclmt::jobsys;
		using rmlv::ivec2, rmlv::vec3;
		auto& vao = buffers_[activeBuffer_];
		vao.clear();

		rmlv::vec2 leftTop{ -0.5F, 0.5F }; // = leftTopNode_ != nullptr ? leftTopNode_->Eval(leftTopSlot_).as_vec2() : leftTop_;
		rmlv::vec2 rightBottom{ 0.5F, -0.5F }; //  = rightBottomNode_ != nullptr ? rightBottomNode_->Eval(rightBottomSlot_).as_vec2() : rightBottom_;
		float z = 0.0F; //zNode_ != nullptr ? zNode_->Eval(zSlot_).as_float() : z_;

		vec3 pul{ leftTop.x, leftTop.y, z };     vec3 pur{ rightBottom.x, leftTop.y, z };
		vec3 tul{ 0.0F, 1.0F, 0 };               vec3 tur{ 1.0F, 1.0F, 0 };

		vec3 pll{ leftTop.x, rightBottom.y, z }; vec3 plr{ rightBottom.x, rightBottom.y, z };
		vec3 tll{ 0.0F, 0.0F, 0 };               vec3 tlr{ 1.0F, 0.0F, 0 };

		vec3 normal{ 0, 0, 1.0F };

		// quad upper left
		vao.append(pur, normal, tur);
		vao.append(pul, normal, tul);
		vao.append(pll, normal, tll);

		// quad lower right
		vao.append(pur, normal, tur);
		vao.append(pll, normal, tll);
		vao.append(plr, normal, tlr);
		vao.pad();

		Job* postSetup = make_job(noop);
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run();}

	void Draw(int pass, rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, int depth [[maybe_unused]]) override {
		auto& dc = *_dc;
		using namespace rglv;
		if (pass != 1) return;
		//dc.Reset();
		std::scoped_lock<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);
		auto [id, ptr] = dc.AllocUniformBuffer<AmyProgram::UniformsSD>();
		dc.UseUniforms(id);

		dc.UseBuffer(0, buffers_[activeBuffer_]);
		dc.DrawArrays(GL_TRIANGLES, 0, 6); }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		// if (!Input("leftTop", /*required=*/true)) { return; }
		// if (!Input("rightBottom", /*required=*/true)) { return; }
		// if (!Input("z", /*required=*/true)) { return; }

		/*vec2 leftTop{ -1.0F, 1.0F };
		vec2 rightBottom{ 1.0F, -1.0F };
		float z{0.0F};*/

		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$plane", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq