#pragma once
#include <cmath>
#include <memory>
#include <string>
#include <string_view>

#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/value.hxx"

namespace rqdq {
namespace rqv {

class CameraNode : public NodeBase {
public:
	CameraNode(std::string_view id, InputList inputs)
		:NodeBase(id, std::move(inputs)) {}
	virtual rmlm::mat4 GetProjectionMatrix(float aspect) const = 0;
	virtual rmlm::mat4 GetModelViewMatrix() const = 0; };


class HandyCamNode : public CameraNode {
public:
	HandyCamNode(std::string_view id, InputList inputs, const rglv::HandyCam& hc)
		:CameraNode(id, std::move(inputs)), handycam(hc) {}
	rmlm::mat4 GetProjectionMatrix(float aspect) const override {
		auto m = rglv::make_gluPerspective(handycam.getFieldOfView(), aspect, 1, 1000);
		//m = rmlm::mat4::translate(-0.425f, 0, 0) * m;
		return m; }
	rmlm::mat4 GetModelViewMatrix() const override{
		return handycam.getMatrix(); }
private:
	const rglv::HandyCam& handycam; };


class ManCamNode : public CameraNode {
public:
	ManCamNode(std::string_view id, InputList inputs, float ha, float va, float fov, rmlv::vec2 origin)
		:CameraNode(id, std::move(inputs)), ha(ha), va(va), fov(fov), origin(origin) {}

	void Connect(std::string_view attr, NodeBase* other, std::string_view slot) override;

	rmlm::mat4 GetProjectionMatrix(float aspect) const override{
		rmlm::mat4 m = rglv::make_gluPerspective(fov, aspect, 1, 1000);
		m = rmlm::mat4::translate(origin.x, origin.y, 0) * m;
		return m; }

	rmlm::mat4 GetModelViewMatrix() const override{
		rmlv::vec3 position = positionNode_->Get(positionSlot_).as_vec3();
		rmlv::vec3 dir = MakeDir();
		rmlv::vec3 right = MakeRight();
		rmlv::vec3 up = cross(right, dir);
		return rglv::look_at(position, position + dir, up); }

private:
	rmlv::vec3 MakeDir() const {
		return { cosf(va)*sinf(ha), sinf(va), cosf(va)*cosf(ha) }; }

	rmlv::vec3 MakeRight() const {
		return { sinf(ha-3.14f/2.0f), 0.0F, cosf(ha-3.14f/2.0f) }; }

private:
	ValuesBase* positionNode_{nullptr};
	std::string positionSlot_{};
	float ha;
	float va;
	float fov;
	rmlv::vec2 origin; };


class OrthographicNode final : public CameraNode {
public:
	OrthographicNode(std::string_view id, InputList inputs)
		:CameraNode(id, std::move(inputs)) {}

	rmlm::mat4 GetProjectionMatrix(float aspect) const override {
		return rglv::make_glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f); }

	rmlm::mat4 GetModelViewMatrix() const override {
		return rmlm::mat4::ident(); }};


}  // namespace rqv
}  // namespace rqdq
