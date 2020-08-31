#include "shaders.hxx"
#include <string_view>

#include "src/rgl/rglr/rglr_fragmentcursor.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_impl.hxx"


namespace rqdq {
namespace rqv {


using namespace std;

ShaderProgramId ShaderProgramNameSerializer::Deserialize(std::string_view text) {
	if (text == "Default") {
		return ShaderProgramId::Default; }
	if (text == "Wireframe") {
		return ShaderProgramId::Wireframe; }
	if (text == "IQ") {
		return ShaderProgramId::IQ; }
	if (text == "Envmap") {
		return ShaderProgramId::Envmap; }
	if (text == "Amy") {
		return ShaderProgramId::Amy; }
	if (text == "Depth") {
		return ShaderProgramId::Depth; }
	if (text == "Many") {
		return ShaderProgramId::Many; }
	if (text == "OBJ1") {
		return ShaderProgramId::OBJ1; }
	if (text == "OBJ2") {
		return ShaderProgramId::OBJ2; }
	if (text == "OBJ2S") {
		return ShaderProgramId::OBJ2S; }
	cout << "can't deserialize shader program \""s << text << "\", using Default"s << endl;
	return ShaderProgramId::Default; }

#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_NOCOLORWRITE       0x22
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND 0x62
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_ALPHA   0x72
#define NOSCISSOR_DEPTH_EQUAL_NODEPTHWRITE_COLORWRITE_ALPHA   0x5a
#define NOSCISSOR_NODEPTH_NODEPTHWRITE_COLORWRITE_ALPHA  0x50

void Install(rglv::GPU& gpu) {
	int id;

	id = static_cast<int>(ShaderProgramId::Default);
	gpu.Install(id, 0, rglv::GPUBltImpl<DefaultPostProgram>::MakeBltProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Wireframe);
	gpu.Install(id, 0, rglv::GPUBinImpl<WireframeProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, WireframeProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::IQ);
	gpu.Install(id, 0, rglv::GPUBltImpl<IQPostProgram>::MakeBltProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Envmap);
	gpu.Install(id, 0, rglv::GPUBinImpl<EnvmapProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_NOCOLORWRITE,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, false, rglv::BlendOff>::MakeFragmentProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_EQUAL_NODEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthEQ, false, true, rglv::BlendAlpha>::MakeFragmentProgramPtrs());
	gpu.Install(id, 0x6e2,
				rglv::GPUTileImpl<rglr::QFloat3FragmentCursor, rglr::QFloatFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, true, rglv::DepthLT, true, true, rglv::BlendAlpha>::MakeFragmentProgramPtrs());
	gpu.Install(id, NOSCISSOR_NODEPTH_NODEPTHWRITE_COLORWRITE_ALPHA,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, EnvmapProgram, false, false, rglv::DepthLT, false, true, rglv::BlendAlpha>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Amy);
	gpu.Install(id, 0, rglv::GPUBinImpl<AmyProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, 0x6e2,
				rglv::GPUTileImpl<rglr::QFloat3FragmentCursor, rglr::QFloatFragmentCursor, AmyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, AmyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Depth);
	gpu.Install(id, 0, rglv::GPUBinImpl<DepthProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, DepthProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::Many);
	gpu.Install(id, 0, rglv::GPUBinImpl<ManyProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, ManyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::OBJ1);
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ1Program>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ1Program, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::OBJ2);
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ2Program>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ2Program, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	id = static_cast<int>(ShaderProgramId::OBJ2S);
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ2SProgram>::MakeVertexProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ2SProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeFragmentProgramPtrs());

	}


}  // namespace rqv
}  // namespace rqdq
