#include "shaders.hxx"
#include <string_view>

#include "src/rgl/rglr/rglr_fragmentcursor.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_gpu_impl.hxx"
#include "src/viewer/shaders_wireframe.hxx"
#include "src/viewer/shaders_envmap.hxx"


namespace rqdq {
namespace rqv {


using namespace std;

int ShaderProgramNameSerializer::Deserialize(std::string_view text) {
	if (text == "Default") {
		return DefaultPostProgram::id; }
	if (text == "Wireframe") {
		return WireframeProgram::id; }
	if (text == "IQ") {
		return IQPostProgram::id; }
	if (text == "Exposure") {
		return ExposurePostProgram::id; }
	if (text == "Envmap") {
		return EnvmapProgram::id; }
	if (text == "Amy") {
		return AmyProgram::id; }
	if (text == "Depth") {
		return DepthProgram::id; }
	if (text == "Many") {
		return ManyProgram::id; }
	if (text == "OBJ1") {
		return OBJ1Program::id; }
	if (text == "OBJ2") {
		return OBJ2Program::id; }
	if (text == "OBJ2S") {
		return OBJ2SProgram::id; }
	cout << "can't deserialize shader program \""s << text << "\", using Default"s << endl;
	return DefaultPostProgram::id; }

#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_NOCOLORWRITE       0x22
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND 0x62
#define NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_ALPHA   0x72
#define NOSCISSOR_DEPTH_EQUAL_NODEPTHWRITE_COLORWRITE_ALPHA   0x5a
#define NOSCISSOR_NODEPTH_NODEPTHWRITE_COLORWRITE_ALPHA  0x50

void Install(rglv::GPU& gpu) {
	int id;

	id = DefaultPostProgram::id;
	gpu.Install(id, 0, rglv::GPUBltImpl<DefaultPostProgram>::MakeBltProgramPtrs());

	id = ExposurePostProgram::id;
	gpu.Install(id, 0, rglv::GPUBltImpl<ExposurePostProgram>::MakeBltProgramPtrs());

	InstallWireframe(gpu);
	InstallEnvmap(gpu);

	id = IQPostProgram::id;
	gpu.Install(id, 0, rglv::GPUBltImpl<IQPostProgram>::MakeBltProgramPtrs());

	id = AmyProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<AmyProgram>::MakeBinProgramPtrs());
	gpu.Install(id, 0x6e2,
				rglv::GPUTileImpl<rglr::QFloat3FragmentCursor, rglr::QFloatFragmentCursor, AmyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, AmyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	id = DepthProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<DepthProgram>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, DepthProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	id = ManyProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<ManyProgram>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, ManyProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	id = OBJ1Program::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ1Program>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ1Program, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	id = OBJ2Program::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ2Program>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ2Program, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	id = OBJ2SProgram::id;
	gpu.Install(id, 0, rglv::GPUBinImpl<OBJ2SProgram>::MakeBinProgramPtrs());
	gpu.Install(id, NOSCISSOR_DEPTH_LESS_DEPTHWRITE_COLORWRITE_NOBLEND,
				rglv::GPUTileImpl<rglr::QFloat4RGBFragmentCursor, rglr::QFloat4AFragmentCursor, OBJ2SProgram, false, true, rglv::DepthLT, true, true, rglv::BlendOff>::MakeDrawProgramPtrs());

	}


}  // namespace rqv
}  // namespace rqdq
