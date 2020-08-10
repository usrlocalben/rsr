#include "uicamera.hxx"

#include <string_view>

#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"

namespace rqdq {
namespace rqv {

UICamera::UICamera(std::string_view id, InputList inputs, const rglv::HandyCam& hc)
	:ICamera(id, std::move(inputs)), hc_(hc) {}


rmlm::mat4 UICamera::ProjectionMatrix(float aspect) const {
	return rglv::Perspective2(hc_.FieldOfView(), aspect, 10, 1000); }


rmlm::mat4 UICamera::ViewMatrix() const {
	return hc_.ViewMatrix(); }


}  // namespace rqv
}  // namespace rqdq
