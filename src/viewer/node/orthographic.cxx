#include <memory>
#include <utility>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_camera.hxx"

namespace rqdq {
namespace {

using namespace rqv;

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

		float b{-1.0F};
		if (auto jv = jv_find(data_, "b", JSON_NUMBER)) {
			b = static_cast<float>(jv->toNumber()); }

		float t{1.0F};
		if (auto jv = jv_find(data_, "t", JSON_NUMBER)) {
			t = static_cast<float>(jv->toNumber()); }

		float l{-1.0F};
		if (auto jv = jv_find(data_, "l", JSON_NUMBER)) {
			l = static_cast<float>(jv->toNumber()); }

		float r{1.0F};
		if (auto jv = jv_find(data_, "r", JSON_NUMBER)) {
			r = static_cast<float>(jv->toNumber()); }

		float n{1.0F};
		if (auto jv = jv_find(data_, "n", JSON_NUMBER)) {
			n = static_cast<float>(jv->toNumber()); }

		float f{-1.0F};
		if (auto jv = jv_find(data_, "f", JSON_NUMBER)) {
			f = static_cast<float>(jv->toNumber()); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), l, r, b, t, n, f); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$orthographic", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
