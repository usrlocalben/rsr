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
public:
	using IGl::IGl;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "leftTop") {
			leftTopNode_ = dynamic_cast<IValue*>(other);
			leftTopSlot_ = slot;
			if (leftTopNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "rightBottom") {
			rightBottomNode_ = dynamic_cast<IValue*>(other);
			rightBottomSlot_ = slot;
			if (rightBottomNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "z") {
			zNode_ = dynamic_cast<IValue*>(other);
			zSlot_ = slot;
			if (zNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void AddDeps() override {
		AddDep(materialNode_);
		AddDep(leftTopNode_);
		AddDep(rightBottomNode_);
		AddDep(zNode_); }

	void Reset() override {
		activeBuffer_ = (activeBuffer_+1)%3;
		IGl::Reset(); }

	void Main() override {
		using namespace rclmt::jobsys;
		using rmlv::ivec2, rmlv::vec3;
		auto& vao = buffers_[activeBuffer_];
		vao.clear();

		rmlv::vec2 leftTop = leftTopNode_ != nullptr ? leftTopNode_->Eval(leftTopSlot_).as_vec2() : leftTop_;
		rmlv::vec2 rightBottom = rightBottomNode_ != nullptr ? rightBottomNode_->Eval(rightBottomSlot_).as_vec2() : rightBottom_;
		float z = zNode_ != nullptr ? zNode_->Eval(zSlot_).as_float() : z_;

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

	void Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) override {
		auto& dc = *_dc;
		using namespace rglv;
		std::scoped_lock<std::mutex> lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		auto [id, ptr] = dc.AllocUniformBuffer<AmyProgram::UniformsSD>();
		ptr->pm = rglv::make_glOrtho(-1.0F, 1.0F, -1.0F, 1.0F, 1.0, -1.0);
		ptr->mvm = rmlm::mat4::ident();
		ptr->nm = transpose(inverse(ptr->mvm));
		ptr->mvpm = ptr->pm * ptr->mvm;
		dc.UseUniforms(id);

		dc.UseBuffer(0, buffers_[activeBuffer_]);
		dc.DrawArrays(GL_TRIANGLES, 0, 6);
		if (link != nullptr) {
			rclmt::jobsys::run(link); } }

private:
	int activeBuffer_{0};
	std::array<rglv::VertexArray_F3F3F3, 3> buffers_;
	rcls::vector<uint16_t> meshIdx_;

	// connections
	IMaterial* materialNode_{nullptr};
	IValue* leftTopNode_{nullptr};
	std::string leftTopSlot_{};
	IValue* rightBottomNode_{nullptr};
	std::string rightBottomSlot_{};
	IValue* zNode_{nullptr};
	std::string zSlot_{};

	// config
	rmlv::vec2 leftTop_;
	rmlv::vec2 rightBottom_;
	float z_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("leftTop", /*required=*/true)) { return; }
		if (!Input("rightBottom", /*required=*/true)) { return; }
		if (!Input("z", /*required=*/true)) { return; }

		/*vec2 leftTop{ -1.0F, 1.0F };
		vec2 rightBottom{ 1.0F, -1.0F };
		float z{0.0F};*/

		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$fxXYQuad", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
