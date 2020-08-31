#include "src/rgl/rglv/rglv_mesh.hxx"

#include <iostream>
#include <string>
#include <unordered_map>

#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rml/rmlv/rmlv_math.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace {



}  // close unnamed namespace

namespace rglv {

using rmlv::vec3;
using std::cout;
using std::endl;
using rclr::len;

							// --------------
							// class Material
							// --------------

void Material::Print() const {
	std::cout << "material[" << name << "]:" << std::endl;
	std::cout << "  ka" << ka << ", ";
	std::cout << "kd" << kd << ", ";
	std::cout << "ks" << ks << std::endl;
	std::cout << "  specpow(" << specpow << "), density(" << d << ")" << std::endl;
	std::cout << "  texture[" << imagename << "]" << std::endl; }


							// ----------
							// class Mesh
							// ----------

void Mesh::Print() const {
	cout << "mesh[" << name_ << "]:" << endl;
	cout << "  vert(" << len(position_) << "), normal(" << normal_.size() << "), uv(" << texture_.size() << ")" << endl;
	cout << "  faces(" << len(face_) << ")" << endl; }


void Mesh::ComputeBBox() {

	vec3 pmin = position_[0];
	vec3 pmax = pmin;
	for (const auto& p : position_) {
		pmin = vmin(pmin, p);
		pmax = vmax(pmax, p); }

	aabb_[0] = { pmin.x, pmin.y, pmin.z, 1 };
	aabb_[1] = { pmin.x, pmin.y, pmax.z, 1 };
	aabb_[2] = { pmin.x, pmax.y, pmin.z, 1 };
	aabb_[3] = { pmin.x, pmax.y, pmax.z, 1 };
	aabb_[4] = { pmax.x, pmin.y, pmin.z, 1 };
	aabb_[5] = { pmax.x, pmin.y, pmax.z, 1 };
	aabb_[6] = { pmax.x, pmax.y, pmin.z, 1 };
	aabb_[7] = { pmax.x, pmax.y, pmax.z, 1 }; }


void Mesh::ComputeFaceAndVertexNormals() {

	smoothNormal_.clear();
	smoothNormal_.resize(position_.size(), vec3{0.0F});

	for (auto& f : face_) {
		const auto i0 = f.idx.position[0];
		const auto i1 = f.idx.position[1];
		const auto i2 = f.idx.position[2];

		const auto v0v2 = position_[i2] - position_[i0];
		const auto v0v1 = position_[i1] - position_[i0];
		const auto dir = cross(v0v1, v0v2);
		f.normal = normalize(dir);

		smoothNormal_[i0] += dir;
		smoothNormal_[i1] += dir;
		smoothNormal_[i2] += dir; }

	for (auto& n : smoothNormal_) {
		n = normalize(n); }}


void Mesh::ComputeEdges() {
	struct _edgedata { int f1, f2, id; };
	std::unordered_map<int, _edgedata> edges;

	auto make_edge_key = [](int v1, int v2) -> int {
		if (v1 > v2) {
			std::swap(v1, v2); }
		return (v1 << 16) | v2; };

	int seq = 0;
	auto generate_edge_id = [&]() { return seq++; };

	solid_ = true;

	/*
	 * pass 1
	 * create a table of unique edges, and the two faces that share them
	 */
	for (int fi=0; fi<len(face_); ++fi) {
		Face& face = face_[fi];
		for (int vi=0; vi<3; ++vi) {
			const int nextVi = (vi+1) % 3;
			const int edgeKey = make_edge_key(face.idx.position[vi],
			                                  face.idx.position[nextVi]);
			if (auto existing = edges.find(edgeKey); existing == end(edges)) {
				// first time this edge has been seen
				const int eid = generate_edge_id();
				face.edge[vi] = eid;
				edges[edgeKey] = _edgedata{ fi, -1, eid }; }
			else {
				// second time this edge has been seen
				auto& edge = existing->second;
				if (edge.f2 != -1) {
					// oops, we saw it more than twice
					solid_ = false;
					message_ = "edge over-shared";
					return; }
				edge.f2 = fi;
				face.edge[vi] = edge.id; }}}

	/*
	 * pass 2
	 * update each face to include the adjacent face for each edge
	 */
	for (int fi=0; fi<len(face_); ++fi) {
		auto& face = face_[fi];
		for (int vi=0; vi<3; ++vi) {
			const int nextVi = (vi+1) % 3;
			const int edgeKey = make_edge_key(face.idx.position[vi],
			                                  face.idx.position[nextVi]);
			auto& edge = edges[edgeKey];
			if (edge.f2 == -1) {
				// oops, this edge has no adjacent face
				solid_ = false;
				message_ = "open edge";
				return; }

			// my neighbor is the face_id that isn't myself
			const int adjacent_fid = edge.f1 == fi ? edge.f2 : edge.f1;
			face.adjacent[vi] = adjacent_fid; }}

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


}  // close package namespace
}  // close enterprise namespace
