/**
 * fps-style camera, as described in
 * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
 *
 * todo: velocity/acceleration
 */
#pragma once
#include <algorithm>
#include <iostream>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rgl/rglv/rglv_math.hxx"

namespace rqdq {
namespace rglv {

class HandyCam {
public:
	HandyCam() = default;

	/**
	 * update camera angle using mouse/pixel delta
	 *
	 * vertical angle is clamped to prevent spinning
	 * upside-down
	 */
	void onMouseMove(rmlv::vec2 delta) {
		angleInRadians_ += mouseSpeedMagic_ * delta;
		float halfPi{ rmlv::M_PI / 2.0F };
		angleInRadians_.y = std::clamp(angleInRadians_.y, -halfPi, halfPi); }

public:
	rmlm::mat4 ViewMatrix() const {
		auto dir = DirectionVector();
		auto right = RightVector();
		auto up = cross(right, dir);
		return LookAt(eyePosition_, eyePosition_ + dir, up); }

	void moveForward()  { eyePosition_ += DirectionVector(); }
	void moveBackward() { eyePosition_ -= DirectionVector(); }
	void moveLeft()     { eyePosition_ -= RightVector(); }
	void moveRight()    { eyePosition_ += RightVector(); }
	void moveUp()       { eyePosition_ += rmlv::vec3{ 0, 0.5F, 0 }; }
	void moveDown()     { eyePosition_ -= rmlv::vec3{ 0, 0.5F, 0 }; }

	float FieldOfView() const { return fieldOfViewInDegrees_; }
	void Zoom(int ticks) { fieldOfViewInDegrees_ += float(ticks); }

	void Print() const {
		std::cout << "<HandyCam eye=" << eyePosition_ << ", angle=" << angleInRadians_ << ", fov=" << fieldOfViewInDegrees_ << ">"; }

private:
	rmlv::vec3 DirectionVector() const {
		auto x = cos(angleInRadians_.y) * sin(angleInRadians_.x);
		auto y = sin(angleInRadians_.y);
		auto z = cos(angleInRadians_.y) * cos(angleInRadians_.x);
		return rmlv::vec3{ x, y, z }; }

	rmlv::vec3 RightVector() const {
		auto x = sin(angleInRadians_.x - 3.14F / 2.0F);
		auto y = 0.0F;
		auto z = cos(angleInRadians_.x - 3.14F / 2.0F);
		return rmlv::vec3{ x, y, z }; }

	const double mouseSpeedMagic_{0.010};
	rmlv::vec3 eyePosition_{ 0, 0, 5 };
	rmlv::vec2 angleInRadians_{ 3.14, 0 };
	double fieldOfViewInDegrees_{ 45.0 }; };


}  // namespace rglv
}  // namespace rqdq
