#include "src/rgl/rglv/rglv_mesh_util.hxx"

#include <string>
#include <tuple>
#include <unordered_map>

#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/tuple_hash/tuple_hash.hxx"

namespace rqdq {
namespace rglv {

using rmlv::vec3;
using std::endl;
using rclr::len;


// FREE FUNCTIONS

void MakeArray(const Mesh& m, const std::string& spec, VertexArray_F3F3& buffer, rcls::vector<uint16_t>& idx) {
	using vertexdata = std::tuple<vec3, vec3>;
	std::unordered_map<vertexdata, int> vertexMap;

	for (const auto& face : m.face_) {
		const auto& material = m.material_[face.frontMaterial];
		for (int i = 0; i < 3; i++) {
			vec3 d[2];

			for (int ti = 0; ti < 2; ++ti) {
				vec3& dd = d[ti];
				switch (spec[ti]) {
				case 'P':
					dd = m.position_[face.idx.position[i]];
					break;
				case 'N':
					dd = m.normal_[face.idx.normal[i]];
					break;
				case 'T':
					dd = vec3{ m.texture_[face.idx.texture[i]], 0 };
					break;
				case 'F':
					dd = face.normal;
					break;
				case 'A':
					dd = material.ka;
					break;
				case 'D':
					dd = material.kd;
					break;
				case 'S':
					dd = material.ks;
					break;

				case 'c':
					dd = m.smoothNormal_[face.idx.position[i]];
					break;
				case 'b': {
					const auto& faceted = face.normal;
					const auto& smooth = m.smoothNormal_[face.idx.position[i]];
					dd = normalize(mix(smooth, faceted, 0.666F)); }
					break;
				default:
					throw std::runtime_error(std::string{"unknown spec "} + spec[ti]); } }

			auto key = vertexdata{ d[0], d[1] };
			int vertexIdx;
			if (auto found = vertexMap.find(key); found != vertexMap.end()) {
				vertexIdx = found->second; }
			else {
				vertexIdx = vertexMap[key] = buffer.append(d[0], d[1]); }
			assert(0 <= vertexIdx && vertexIdx < 65536);
			idx.push_back(static_cast<uint16_t>(vertexIdx)); }}

	buffer.pad(); }


void MakeArray(const Mesh& m, const std::string& spec, VertexArray_F3F3F3& buffer, rcls::vector<uint16_t>& idx) {
	using vertexdata = std::tuple<vec3, vec3, vec3>;
	std::unordered_map<vertexdata, int> vertexMap;

	for (const auto& face : m.face_) {
		const auto& material = m.material_[face.frontMaterial];
		for (int i = 0; i < 3; i++) {
			vec3 d[3];

			for (int ti = 0; ti < 3; ++ti) {
				vec3& dd = d[ti];
				switch (spec[ti]) {
				case 'P':
					dd = m.position_[face.idx.position[i]];
					break;
				case 'N':
					dd = m.normal_[face.idx.normal[i]];
					break;
				case 'T':
					dd = vec3{ m.texture_[face.idx.texture[i]], 0 };
					break;
				case 'F':
					dd = face.normal;
					break;
				case 'A':
					dd = material.ka;
					break;
				case 'D':
					dd = material.kd;
					break;
				case 'S':
					dd = material.ks;
					break;

				case 'c':
					dd = m.smoothNormal_[face.idx.position[i]];
					break;
				case 'b': {
					const auto& faceted = face.normal;
					const auto& smooth = m.smoothNormal_[face.idx.position[i]];
					dd = normalize(mix(smooth, faceted, 0.666F)); }
					break;
				default:
					throw std::runtime_error(std::string{"unknown spec "} + spec[ti]); } }

			vertexdata key{ d[0], d[1], d[2] };
			int vertexIdx;
			if (auto found = vertexMap.find(key); found != vertexMap.end()) {
				vertexIdx = found->second; }
			else {
				vertexIdx = vertexMap[key] = buffer.append(d[0], d[1], d[2]); }
			assert(0 <= vertexIdx && vertexIdx < 65536);
			idx.push_back(static_cast<uint16_t>(vertexIdx)); }}

	buffer.pad(); }


}  // close package namespace
}  // close enterprise namespace
