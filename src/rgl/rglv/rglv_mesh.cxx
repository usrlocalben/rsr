#include <rglv_mesh.hxx>

#include <rclr_algorithm.hxx>
#include <rglv_vao.hxx>
#include <rmlv_vec.hxx>

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>

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


std::tuple<VertexArray_PN, rcls::vector<uint16_t>> make_indexed_vao_PN(const Mesh& m) {
	VertexArray_PN d;
	rcls::vector<uint16_t> idx;

	for (const auto& face : m.faces) {
		for (int i = 0; i < 3; i++) {
			auto point = m.points[face.point_idx[i]];
			// auto normal = m.normals[face.normal_idx[i]];
			auto smooth = m.vertex_normals[face.point_idx[i]];
			auto faceted = face.normal;
			auto normal = normalize(lerp(smooth, faceted, 0.666));
			// auto texcoord = m.texcoords[face.texcoord_idx[i]];
			int vao_idx = d.upsert(point, normal); // , texcoord);
			idx.push_back(vao_idx); }}

	d.pad();
	return std::tuple{d, idx}; }


std::tuple<VertexArray_PNM, rcls::vector<uint16_t>> make_indexed_vao_PNM(const Mesh& m) {
	VertexArray_PNM d;
	rcls::vector<uint16_t> idx;

	for (const auto& face : m.faces) {
		for (int i = 0; i < 3; i++) {
			int vao_idx = d.upsert(

				// position
				m.points[face.point_idx[i]],

				// computed smooth normals
				m.vertex_normals[face.point_idx[i]],

				// computed face normals
				face.normal

				// blended normals
				// normalize(lerp(smooth, faceted, 0.666)),

				// obj normals
				// m.normals[face.normal_idx[i]],


				// obj texture coords
				// vec3{ m.texcoords[face.texcoord_idx[i]], 0 },
				);

			idx.push_back(vao_idx); }}

	d.pad();
	return std::tuple{d, idx}; }


}  // close package namespace
}  // close enterprise namespace
