#include "src/rgl/rglv/rglv_mesh.hxx"

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>

#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/tuple_hash/tuple_hash.hxx"

namespace rqdq {
namespace rglv {

using rmlv::vec3;
using std::cout;
using std::endl;
using rclr::len;


void Mesh::print() const {
	cout << "mesh[" << this->name << "]:" << endl;
	cout << "  vert(" << len(this->points) << "), normal(" << this->normals.size() << "), uv(" << this->texcoords.size() << ")" << endl;
	cout << "  faces(" << len(this->faces) << ")" << endl; }


void Mesh::compute_bbox() {

	vec3 pmin = this->points[0];
	vec3 pmax = pmin;
	for (auto& item : this->points) {
		pmin = vmin(pmin, item);
		pmax = vmax(pmax, item); }

	bbox[0] = { pmin.x, pmin.y, pmin.z, 1 };
	bbox[1] = { pmin.x, pmin.y, pmax.z, 1 };
	bbox[2] = { pmin.x, pmax.y, pmin.z, 1 };
	bbox[3] = { pmin.x, pmax.y, pmax.z, 1 };
	bbox[4] = { pmax.x, pmin.y, pmin.z, 1 };
	bbox[5] = { pmax.x, pmin.y, pmax.z, 1 };
	bbox[6] = { pmax.x, pmax.y, pmin.z, 1 };
	bbox[7] = { pmax.x, pmax.y, pmax.z, 1 }; }


void Mesh::compute_face_and_vertex_normals() {

	vertex_normals.clear();
	for (int i = 0; i < len(points); i++) {
		vertex_normals.push_back(vec3{ 0 }); }

	for (auto& face : faces) {
		vec3 v0v2 = points[face.point_idx[2]] - points[face.point_idx[0]];
		vec3 v0v1 = points[face.point_idx[1]] - points[face.point_idx[0]];
		vec3 dir = cross(v0v1, v0v2);
		face.normal = normalize(dir);

		vertex_normals[face.point_idx[0]] += dir;
		vertex_normals[face.point_idx[1]] += dir;
		vertex_normals[face.point_idx[2]] += dir; }

	for (auto& vn : vertex_normals) {
		vn = normalize(vn); }}


struct _edgedata {
	int first_face_id;
	int second_face_id;
	int edge_id; };


int make_edge_key(const int vidx1, const int vidx2) {
	int first, second;
	if (vidx1 < vidx2) {
		first = vidx1;
		second = vidx2; }
	else {
		first = vidx2;
		second = vidx1; }
	return (first << 16) | second; }


void Mesh::compute_edges() {

	std::map<int, _edgedata> edges;
	int edge_id_seq = 0;
	auto generate_edge_id = [&]() {
		return edge_id_seq++;
	};

	solid = true;

	/*
	 * pass 1
	 * create a table of unique edges, and the two faces that share them
	 */
	for (int face_idx = 0; face_idx < len(faces); face_idx += 1) {
		Face& face = faces[face_idx];
		for (int vertex_idx = 0; vertex_idx < 3; vertex_idx += 1) {
			const int next_vertex_idx = (vertex_idx + 1) % 3;
			const int edgekey = make_edge_key(face.point_idx[vertex_idx],
												   face.point_idx[next_vertex_idx]);
			if (edges.count(edgekey) == 0) {
				// first time this edge has been seen
				int this_edge_id = generate_edge_id();
				face.edge_ids[vertex_idx] = this_edge_id;
				edges[edgekey] = _edgedata{ face_idx, -1, this_edge_id }; }
			else {
				// second time this edge has been seen
				if (edges[edgekey].second_face_id != -1) {
					// oops, we saw it more than twice
					solid = false;
					message = "edge over shared";
					return; }
				edges[edgekey].second_face_id = face_idx;
				face.edge_ids[vertex_idx] = edges[edgekey].edge_id; }}}

	/*
	 * pass 2
	 * update each face to include the adjacent face for each edge
	 */
	for (int face_idx = 0; face_idx < len(faces); face_idx += 1) {
		auto& face = faces[face_idx];
		for (int vertex_idx=0; vertex_idx<3; vertex_idx++) {
			const int next_vertex_idx = (vertex_idx + 1) % 3;
			const int edgekey = make_edge_key(face.point_idx[vertex_idx],
												   face.point_idx[next_vertex_idx]);
			auto& edge = edges[edgekey];
			if (edge.second_face_id == -1) {
				// oops, this edge has no adjacent face
				solid = false;
				message = "open edge";
				return; }

			// my neighbor is the face_id that isn't myself
			const int adjacent_face_id = edge.first_face_id == face_idx ? edge.second_face_id : edge.first_face_id;
			face.edge_faces[vertex_idx] = adjacent_face_id; }}

	/*
	pass 3
	fill in SoA format point-in-face & face-normals for shadow-volumes
	for (auto& face : faces) {

		const auto& adj1 = faces[face.edge_faces[0]];
		const auto& adj2 = faces[face.edge_faces[1]];
		const auto& adj3 = faces[face.edge_faces[2]];

		// point-in-face for myself, and each adjacent face
		const auto& pi = points[face.point_idx[0]];
		const auto& p1 = points[adj1.point_idx[0]];
		const auto& p2 = points[adj2.point_idx[0]];
		const auto& p3 = points[adj3.point_idx[0]];

		// normal for myself, and each adjacent face
		const auto& ni = face.normal;
		const auto& n1 = adj1.normal;
		const auto& n2 = adj2.normal;
		const auto& n3 = adj3.normal;

		face.px = vec4( pi.x, p1.x, p2.x, p3.x );
		face.py = vec4( pi.y, p1.y, p2.y, p3.y );
		face.pz = vec4( pi.z, p1.z, p2.z, p3.z );

		face.nx = vec4( ni.x, n1.x, n2.x, n3.x );
		face.ny = vec4( ni.y, n1.y, n2.y, n3.y );
		face.nz = vec4( ni.z, n1.z, n2.z, n3.z );
	}*/
}


std::tuple<VertexArray_F3F3, rcls::vector<uint16_t>> make_indexed_vao_F3F3(const Mesh& m) {
	VertexArray_F3F3 vao;
	using vertexdata = std::tuple<vec3, vec3>;
	std::unordered_map<vertexdata, int> vertexMap;
	rcls::vector<uint16_t> idx;

	for (const auto& face : m.faces) {
		for (int i = 0; i < 3; i++) {
			auto d0 = m.points[face.point_idx[i]];
			// auto normal = m.normals[face.normal_idx[i]];
			auto smooth = m.vertex_normals[face.point_idx[i]];
			auto faceted = face.normal;
			auto d1 = normalize(mix(smooth, faceted, 0.666));
			// auto texcoord = m.texcoords[face.texcoord_idx[i]];

			auto key = vertexdata{d0, d1};
			int vertexIdx;
			if (auto found = vertexMap.find(key); found != vertexMap.end()) {
				vertexIdx = found->second; }
			else {
				vertexIdx = vertexMap[key] = vao.append(d0, d1); }
			idx.push_back(vertexIdx); }}

	vao.pad();
	return std::tuple{vao, idx}; }


std::tuple<VertexArray_F3F3F3, rcls::vector<uint16_t>> make_indexed_vao_F3F3F3(const Mesh& m) {
	VertexArray_F3F3F3 vao;
	using vertexdata = std::tuple<vec3, vec3, vec3>;
	std::unordered_map<vertexdata, int> vertexMap;
	rcls::vector<uint16_t> idx;

	for (const auto& face : m.faces) {
		for (int i = 0; i < 3; i++) {
			// position
			auto d0 = m.points[face.point_idx[i]];
			// computed smooth normals
			auto d1 = m.vertex_normals[face.point_idx[i]];
			// computed face normals
			auto d2 = face.normal;

			// blended normals
			// normalize(mix(smooth, faceted, 0.666)),

			// obj normals
			// m.normals[face.normal_idx[i]],

			// obj texture coords
			// vec3{ m.texcoords[face.texcoord_idx[i]], 0 },

			vertexdata key{ d0, d1, d2 };
			int vertexIdx;
			if (auto found = vertexMap.find(key); found != vertexMap.end()) {
				vertexIdx = found->second; }
			else {
				vertexIdx = vertexMap[key] = vao.append(d0, d1, d2); }
			idx.push_back(vertexIdx); }}

	vao.pad();
	return std::tuple{vao, idx}; }


}  // namespace rglv
}  // namespace rqdq
