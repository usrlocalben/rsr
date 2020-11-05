#include "src/rgl/rglv/rglv_camera.hxx"

#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <ostream>
#include <cmath>

namespace rqdq {
namespace {

constexpr float pi_2 = rmlv::M_PI / 2.F;
constexpr float mouseSpeedMagic = 0.010F;


}  // close unnamed namespace
namespace rglv {

void HandyCam::OnMouseMove(rmlv::vec2 delta) {
	angleInRadians_ += mouseSpeedMagic * delta;
	angleInRadians_.y = std::clamp(angleInRadians_.y, -pi_2, pi_2); }


auto HandyCam::Dump(std::ostream& s) const -> std::ostream& {
	s << "<HandyCam eye=" << position_ << ", angle=" << angleInRadians_ << ", fov=" << fieldOfViewInDegrees_ << ">";
	return s; }


auto HandyCam::Heading() const -> rmlv::vec3 {
	auto x = cos(angleInRadians_.y) * sin(angleInRadians_.x);
	auto y = sin(angleInRadians_.y);
	auto z = cos(angleInRadians_.y) * cos(angleInRadians_.x);
	return rmlv::vec3{ x, y, z }; }


auto HandyCam::Right() const -> rmlv::vec3 {
	auto x = sin(angleInRadians_.x - pi_2);
	auto y = 0.0F;
	auto z = cos(angleInRadians_.x - pi_2);
	return rmlv::vec3{ x, y, z }; }


auto HandyCam::Up() const -> rmlv::vec3 {
	return cross(Right(), Heading()); }


auto HandyCam::ViewMatrix() const -> rmlm::mat4 {
	auto dir = Heading();
	auto right = Right();
	auto up = cross(right, dir);
	return LookAt(position_, position_ + Heading(), Up()); }


}  // close package namespace
}  // close enterprise namespace
