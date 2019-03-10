/**
 * fps-style camera, as described in
 * http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
 *
 * todo: velocity/acceleration
 */
#pragma once
#include <iostream>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rgl/rglv/rglv_math.hxx"

namespace rqdq {
namespace rglv {

class HandyCam {
public:
	HandyCam():
		d_position({0,0,5}),
		d_angle({3.14, 0.0}),
		d_field_of_view(45.0),
		d_mouse_speed(0.010)
		{}

	/**
	 * update camera angle using mouse/pixel delta
	 *
	 * vertical angle is clamped to prevent spinning
	 * upside-down
	 */
	void onMouseMove(rmlv::vec2 delta) {
		d_angle += d_mouse_speed * delta;
		d_angle.y = rmlv::clamp(d_angle.y, -(rmlv::M_PI / 2.0f), rmlv::M_PI / 2.0f); }

public:
	rmlm::mat4 getMatrix() const {
		auto dir = makeDirectionVector();
		auto right = makeRightVector();
		auto up = cross(right, dir);
		return look_at(d_position, d_position + dir, up); }

	void moveForward()  { d_position += makeDirectionVector(); }
	void moveBackward() { d_position -= makeDirectionVector(); }
	void moveLeft()     { d_position -= makeRightVector(); }
	void moveRight()    { d_position += makeRightVector(); }
	void moveUp()       { d_position += rmlv::vec3{ 0, 0.5f, 0 }; }
	void moveDown()     { d_position -= rmlv::vec3{ 0, 0.5f, 0 }; }

	float getFieldOfView() const { return d_field_of_view; }
	void adjustZoom(const int ticks) { d_field_of_view += float(ticks); }

	void print() {
		std::cout << "position=" << d_position << ", angle=" << d_angle << ", fov=" << d_field_of_view << std::endl; }

private:
	rmlv::vec3 makeDirectionVector() const {
		auto x = cos(d_angle.y) * sin(d_angle.x);
		auto y = sin(d_angle.y);
		auto z = cos(d_angle.y) * cos(d_angle.x);
		return rmlv::vec3{ x, y, z }; }

	rmlv::vec3 makeRightVector() const {
		auto x = sin(d_angle.x - 3.14f / 2.0f);
		auto y = 0.0f;
		auto z = cos(d_angle.x - 3.14f / 2.0f);
		return rmlv::vec3{ x, y, z }; }

	rmlv::vec3 d_position;
	rmlv::vec3 d_direction;
	rmlv::vec2 d_angle;
	double d_field_of_view;
	double d_mouse_speed;
	};


}  // namespace rglv
}  // namespace rqdq
