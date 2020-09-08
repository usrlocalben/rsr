#include "shaders_envmap.hxx"

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

void InstallEnvmap(rglv::GPU& gpu) {
	int id = EnvmapProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<EnvmapProgram>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_NOCOLORWRITE,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, false, rglv::BlendOff>::MakeDrawProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_EQUAL_NODEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthEQ, false, true, rglv::BlendAlpha>::MakeDrawProgramPtrs());
	gpu.Install(id, 0x6e2,
				rglv::GPUTileImpl<rglr::QFloat3FragmentCursor, rglr::QFloatFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendAlpha>::MakeDrawProgramPtrs());
	gpu.Install(id, NOSCISSOR_NODEPTH_NODEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, false, rglv::DepthLT, false, true, rglv::BlendAlpha>::MakeDrawProgramPtrs()); }


}  // namespace rqv
}  // namespace rqdq

