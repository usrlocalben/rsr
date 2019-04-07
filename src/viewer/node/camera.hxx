#pragma once
#include <string_view>

#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"

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


}  // namespace rqv
}  // namespace rqdq
