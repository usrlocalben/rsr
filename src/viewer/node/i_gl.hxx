#pragma once
#include <string_view>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {


constexpr int kMaxLights = 4;

struct LightPack {
	int cnt{0};
	rmlm::mat4 vmat[kMaxLights];
	float angle[kMaxLights];
	int size[kMaxLights];

	rmlv::vec3 pos[kMaxLights];
	rmlv::vec3 dir[kMaxLights];
	rmlm::mat4 pmat[kMaxLights];
	float cos[kMaxLights];
	float* map[kMaxLights]; };


inline
auto Merge(LightPack a, LightPack b) {
	for (int i = 0; i < b.cnt; ++i) {
		if (a.cnt >= kMaxLights) {
			std::cerr << "some lights were dropped\n";
			break; }
		a.vmat[a.cnt] = b.vmat[i];
		a.angle[a.cnt] = b.angle[i];
		a.size[a.cnt] = b.size[i];
		++a.cnt; }
	return a; }


class IGl : public NodeBase {
public:
	using NodeBase::NodeBase;
	virtual void Draw(int /*pass*/, const LightPack& /*lights*/, rglv::GL* /*dc*/, const rmlm::mat4* /*pmat*/, const rmlm::mat4* /*vmat*/, const rmlm::mat4* /*mmat*/) {}
	virtual void DrawDepth(rglv::GL* /*dc*/, const rmlm::mat4* /*pmat*/, const rmlm::mat4* /*mvmat*/) {}
	virtual auto Lights(rmlm::mat4 mvmat [[maybe_unused]]) -> LightPack { return {}; } };


}  // namespace rqv
}  // namespace rqdq
