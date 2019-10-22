#pragma once
#include <string_view>

#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/viewer/node/i_camera.hxx"

namespace rqdq {
namespace rqv {

class UICamera : public ICamera {
public:
	UICamera(std::string_view id, InputList inputs, const class rglv::HandyCam& hc);
	rmlm::mat4 ProjectionMatrix(float aspect) const override;
	rmlm::mat4 ViewMatrix() const override;

private:
	const class rglv::HandyCam& hc_; };


}  // namespace rqv
}  // namespace rqdq
