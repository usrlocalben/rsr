#include <rqv_shaders.hxx>

namespace rqdq {
namespace rqv {

using namespace std;

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
