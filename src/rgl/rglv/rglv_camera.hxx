/**
 * fps-style camera, as described in
 * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
 *
 * todo: velocity/acceleration
 */
#pragma once
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <algorithm>
#include <iostream>

namespace rqdq {
namespace rglv {

class HandyCam {
	rmlv::vec3 position_{ 0, 3.5F, 14.0F };
	rmlv::vec2 angleInRadians_{ 3.14F, 0 };
	// rmlv::vec3 position_{ 0, 14.0F, 0.0F };
	// rmlv::vec2 angleInRadians_{ 3.14F, -3.14F/2.0F };
	float fieldOfViewInDegrees_{ 45.0F };

public:
	HandyCam() = default;

	// MANIPULATORS

	/**
	 * update camera angle using mouse/pixel delta
	 *
	 * vertical angle is clamped to prevent spinning
	 * upside-down
	 */
	void OnMouseMove(rmlv::vec2 delta);

	void OnMouseWheel(int ticks);

	void MoveForward()  { position_ +=  Heading(); }
	void MoveBackward() { position_ += -Heading(); }
	void MoveLeft()     { position_ += -Right(); }
	void MoveRight()    { position_ +=  Right(); }
	void MoveUp()       { position_ += rmlv::vec3{ 0, 0.5F, 0 }; }
	void MoveDown()     { position_ -= rmlv::vec3{ 0, 0.5F, 0 }; }

	// ACCESSORS
	auto FieldOfView() const -> float;
	auto Heading() const -> rmlv::vec3;
	auto Right() const -> rmlv::vec3;
	auto Up() const -> rmlv::vec3;
	auto ViewMatrix() const -> rmlm::mat4;

	// OSTREAM
	auto Dump(std::ostream& s) const -> std::ostream&; };


inline
void HandyCam::OnMouseWheel(int ticks) {
	fieldOfViewInDegrees_ += float(ticks); }


inline
auto HandyCam::FieldOfView() const -> float {
	return fieldOfViewInDegrees_; }


}  // namespace rglv
}  // namespace rqdq


inline
auto operator<<(std::ostream& os, const rqdq::rglv::HandyCam& a) -> std::ostream& {
	return a.Dump(os); }
