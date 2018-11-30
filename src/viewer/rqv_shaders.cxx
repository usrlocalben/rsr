#include "src/viewer/rqv_shaders.hxx"

namespace rqdq {
namespace rqv {

using namespace std;

int WireframeProgram::id = int(ShaderProgramId::Wireframe);
int IQPostProgram::id = int(ShaderProgramId::IQ);
int EnvmapProgram::id = int(ShaderProgramId::Envmap);
int AmyProgram::id = int(ShaderProgramId::Amy);
int EnvmapXProgram::id = int(ShaderProgramId::EnvmapX);


ShaderProgramId deserialize_program_name(const string& text) {
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
	cout << "can't deserialize shader program \""s << text << "\", using Default"s << endl;
	return ShaderProgramId::Default; }


}  // close package namespace
}  // close enterprise namespace
