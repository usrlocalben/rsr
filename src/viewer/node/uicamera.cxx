#include "uicamera.hxx"

#include <string_view>

#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"

namespace rqdq {
namespace rqv {

UICamera::UICamera(std::string_view id, InputList inputs, const rglv::HandyCam& hc) :
	ICamera(id, std::move(inputs)),
	hc_(hc) {}


auto UICamera::ProjectionMatrix(float aspect) const -> rmlm::mat4 {
	return rglv::Perspective2(hc_.FieldOfView(), aspect, 0.5, 100); }


auto UICamera::ViewMatrix() const -> rmlm::mat4 {
	return hc_.ViewMatrix(); }


}  // close package namespace
}  // close enterprise namespace
