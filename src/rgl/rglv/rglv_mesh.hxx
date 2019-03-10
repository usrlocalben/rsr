#pragma once
#include <string>
#include <tuple>
#include <array>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

struct Face {
	std::array<int, 3> point_idx;
	std::array<int, 3> texcoord_idx;
	std::array<int, 3> normal_idx;
	rmlv::vec3 normal;
	int front_material;
//	int back_material;

	std::array<int, 3> edge_ids;  // edge_id for each edge
	std::array<int, 3> edge_faces;  // adjacent face_id for each edge

	/*
	soa point-in-face & adjacent-normals for shadow volumes
	vec4 nx,ny,nz;
	vec4 px,py,pz;
	*/
	};


struct Mesh {
	rmlv::vec4 bbox[8];

	rcls::vector<rmlv::vec3> points;
	rcls::vector<rmlv::vec3> normals;
	rcls::vector<rmlv::vec2> texcoords;

	rcls::vector<rmlv::vec3> vertex_normals;
	rcls::vector<Face> faces;

	std::string name;

	bool solid;
	std::string message;

	void print() const;
	void compute_bbox();
	void compute_face_and_vertex_normals();
	void compute_edges();
	};


/*inline bool almost_equal(const vao_vertex& a, const vao_vertex& b) {
	return (
		almost_equal(a.point, b.point) &&
		almost_equal(a.normal, b.normal) &&
		almost_equal(a.texcoord, b.texcoord)); }*/


std::tuple<rglv::VertexArray_F3F3, rcls::vector<uint16_t>> make_indexed_vao_F3F3(const Mesh&);
std::tuple<rglv::VertexArray_F3F3F3, rcls::vector<uint16_t>> make_indexed_vao_F3F3F3(const Mesh&);


}  // namespace rglv
}  // namespace rqdq
