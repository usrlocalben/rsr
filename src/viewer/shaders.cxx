#include "shaders.hxx"
#include <string_view>

#include "src/rgl/rglv/rglv_gpu.hxx"


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
	if (text == "EnvmapX") {
		return ShaderProgramId::EnvmapX; }
	if (text == "OBJ1") {
		return ShaderProgramId::OBJ1; }
	if (text == "OBJ2") {
		return ShaderProgramId::OBJ2; }
	if (text == "Many") {
		return ShaderProgramId::Many; }
	cout << "can't deserialize shader program \""s << text << "\", using Default"s << endl;
	return ShaderProgramId::Default; }


void Install(rglv::GPU& gpu) {
	{
		auto id = static_cast<int>(ShaderProgramId::Wireframe);
		gpu.Install(id, 0, rglv::GPUBinImpl<WireframeProgram>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<WireframeProgram, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<WireframeProgram, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::IQ);
		gpu.Install(id, 0, rglv::GPUBltImpl<IQPostProgram>::MakeBltProgramPtrs());}


	{
		auto id = static_cast<int>(ShaderProgramId::Envmap);
		gpu.Install(id, 0, rglv::GPUBinImpl<EnvmapProgram>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<EnvmapProgram, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<EnvmapProgram, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::Amy);
		gpu.Install(id, 0, rglv::GPUBinImpl<AmyProgram>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<AmyProgram, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<AmyProgram, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::EnvmapX);
		gpu.Install(id, 0, rglv::GPUBinImpl<EnvmapXProgram>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<EnvmapXProgram, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<EnvmapXProgram, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::OBJ1);
		gpu.Install(id, 0, rglv::GPUBinImpl<OBJ1Program>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<OBJ1Program, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<OBJ1Program, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::OBJ2);
		gpu.Install(id, 0, rglv::GPUBinImpl<OBJ2Program>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<OBJ2Program, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<OBJ2Program, true>::MakeFragmentProgramPtrs());}

	{
		auto id = static_cast<int>(ShaderProgramId::Many);
		gpu.Install(id, 0, rglv::GPUBinImpl<ManyProgram>::MakeVertexProgramPtrs());
		gpu.Install(id, 0, rglv::GPUTileImpl<ManyProgram, false>::MakeFragmentProgramPtrs());
		gpu.Install(id, 1, rglv::GPUTileImpl<ManyProgram, true>::MakeFragmentProgramPtrs());}

}


}  // namespace rqv
}  // namespace rqdq
