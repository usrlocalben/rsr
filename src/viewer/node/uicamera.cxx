#include "uicamera.hxx"

#include <string_view>

#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"

namespace rqdq {
namespace rqv {

UICamera::UICamera(std::string_view id, InputList inputs, const rglv::HandyCam& hc)
	:ICamera(id, std::move(inputs)), hc_(hc) {}


rmlm::mat4 UICamera::GetProjectionMatrix(float aspect) const {
	auto m = rglv::make_gluPerspective(hc_.getFieldOfView(), aspect, 1, 1000);
	return m; }


rmlm::mat4 UICamera::GetModelViewMatrix() const {
	return hc_.getMatrix(); }


}  // namespace rqv
}  // namespace rqdq
