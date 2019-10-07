#include "shaders.hxx"

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
	if (text == "Many") {
		return ShaderProgramId::Many; }
	cout << "can't deserialize shader program \""s << text << "\", using Default"s << endl;
	return ShaderProgramId::Default; }


}  // namespace rqv
}  // namespace rqdq
