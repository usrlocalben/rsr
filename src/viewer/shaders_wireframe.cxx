#include "shaders_wireframe.hxx"

#include "src/rgl/rglr/rglr_fragmentcursor.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_impl.hxx"

namespace rqdq {
namespace rqv {

using namespace std;

#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_NOCOLORWRITE       0x22
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND 0x62
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_ALPHA   0x72
#define NOSCISSOR_DEPTH_EQUAL_NODEPTHWRITE_COLORWRITE_ALPHA   0x5a
#define NOSCISSOR_NODEPTH_NODEPTHWRITE_COLORWRITE_ALPHA  0x50

void InstallWireframe(rglv::GPU& gpu) {
	auto id = WireframeProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<WireframeProgram>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, WireframeProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());
	}


}  // namespace rqv
}  // namespace rqdq
