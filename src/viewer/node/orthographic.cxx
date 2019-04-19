#include <memory>
#include <utility>

#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_camera.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl final : public ICamera {
public:
	using ICamera::ICamera;
	rmlm::mat4 GetProjectionMatrix(float aspect) const override {
		return rglv::make_glOrtho(-1.0F, 1.0F, -1.0F, 1.0F, 1.0F, -1.0F); }

	rmlm::mat4 GetModelViewMatrix() const override {
		return rmlm::mat4::ident(); }};


class Compiler final : public NodeCompiler {
	void Build() override {
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$orthographic", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
