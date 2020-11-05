#pragma once
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/viewer/node/i_camera.hxx"

#include <string_view>

namespace rqdq {
namespace rqv {

class UICamera : public ICamera {
	const class rglv::HandyCam& hc_;

public:
	UICamera(std::string_view id, InputList inputs, const class rglv::HandyCam& hc);

	// --- ICamera ---
	auto ProjectionMatrix(float aspect) const -> rmlm::mat4 override;
	auto ViewMatrix() const -> rmlm::mat4 override; };


}  // close package namespace
}  // close enterprise namespace
