#pragma once
#include <rglv_camera.hxx>
#include <rglv_math.hxx>
#include <rmlm_mat4.hxx>
#include <rmlv_vec.hxx>
#include <rqv_node_base.hxx>
#include <rqv_node_value.hxx>

#include <memory>
#include <string>

namespace rqdq {
namespace rqv {

struct CameraNode : public NodeBase {
	CameraNode(const std::string& name, const InputList& inputs) :NodeBase(name, inputs) {}
	virtual rmlm::mat4 getProjectionMatrix(float aspect) const = 0;
	virtual rmlm::mat4 getModelViewMatrix() const = 0; };


struct HandyCamNode : public CameraNode {
	const rglv::HandyCam& handycam;
	HandyCamNode(const std::string& name, const InputList& inputs, const rglv::HandyCam& hc) :CameraNode(name, inputs), handycam(hc) {}
	rmlm::mat4 getProjectionMatrix(float aspect) const override {
		auto m = rglv::make_gluPerspective(handycam.getFieldOfView(), aspect, 1, 1000);
		//m = rmlm::mat4::translate(-0.425f, 0, 0) * m;
		return m; }
	rmlm::mat4 getModelViewMatrix() const override{
		return handycam.getMatrix(); }};


struct ManCamNode : public CameraNode {
	ManCamNode(const std::string& name, const InputList& inputs, float ha, float va, float fov, rmlv::vec2 origin) :CameraNode(name, inputs), ha(ha), va(va), fov(fov), origin(origin) {}
	virtual void connect(const std::string&, NodeBase*, const std::string&);

	ValuesBase* position_node = nullptr;
	std::string position_slot;
	const float ha;
	const float va;
	const float fov;
	const rmlv::vec2 origin;

	rmlv::vec3 makeDir() const {
		return rmlv::vec3{
			cos(va) * sin(ha),
			sin(va),
			cos(va) * cos(ha) }; }

	rmlv::vec3 makeRight() const {
		return rmlv::vec3{
			sin(ha - 3.14f / 2.0f),
			0,
			cos(ha - 3.14f / 2.0f) }; }

	rmlm::mat4 getProjectionMatrix(float aspect) const override{
		rmlm::mat4 m = rglv::make_gluPerspective(fov, aspect, 1, 1000);
		m = rmlm::mat4::translate(origin.x, origin.y, 0) * m;
		return m; }

	rmlm::mat4 getModelViewMatrix() const override{
		rmlv::vec3 position = position_node->get(position_slot).as_vec3();
		rmlv::vec3 dir = makeDir();
		rmlv::vec3 right = makeRight();
		rmlv::vec3 up = cross(right, dir);
		return rglv::look_at(position, position + dir, up); }};


struct OrthographicNode : public CameraNode {
	OrthographicNode(const std::string& name, const InputList& inputs) :CameraNode(name, inputs) {}

	rmlm::mat4 getProjectionMatrix(float aspect) const override {
		return rglv::make_glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f); }

	rmlm::mat4 getModelViewMatrix() const override {
		return rmlm::mat4::ident(); }};

}  // close package namespace
}  // close enterprise namespace
