#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_camera.hxx"

#include <memory>
#include <utility>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl final : public ICamera {

	const rmlm::mat4 m_;

public:
	Impl(std::string_view id, InputList inputs, float l, float r, float b, float t, float n, float f) :
		ICamera(id, std::move(inputs)),
		m_(rglv::Orthographic(l, r, b, t, n, f)) {}

	// using ICamera::ICamera;
	auto ProjectionMatrix(float aspect [[maybe_unused]]) const -> rmlm::mat4 override {
		return m_; }

	auto ViewMatrix() const -> rmlm::mat4 override {
		return rmlm::mat4{1}; }};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;

		auto b = DataReal("b", -1.0F);
		auto t = DataReal("t",  1.0F);
		auto l = DataReal("l", -1.0F);
		auto r = DataReal("r",  1.0F);
		auto n = DataReal("n",  1.0F);
		auto f = DataReal("f", -1.0F);

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), l, r, b, t, n, f); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$orthographic", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // close unnamed namespace
