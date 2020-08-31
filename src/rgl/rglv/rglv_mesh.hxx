#pragma once
#include <array>
#include <string>
#include <vector>

#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

struct Material {
	rmlv::vec3 ka;
	rmlv::vec3 kd;
	rmlv::vec3 ks;
	float specpow;
	float d;
	int pass;
	int invert;
	std::string name;
	std::string imagename;

	void Print() const; };


struct Face {
	struct {
		std::array<int, 3> position;
		std::array<int, 3> normal;
		std::array<int, 3> texture;
	} idx;

	rmlv::vec3 normal;
	int frontMaterial;
//	int back_material;

	std::array<int, 3> edge;      // edge_id for each edge
	std::array<int, 3> adjacent;  // adjacent face_id for each edge

	/*
	soa point-in-face & adjacent-normals for shadow volumes
	vec4 nx,ny,nz;
	vec4 px,py,pz;
	*/
	};


struct Mesh {
	std::array<rmlv::vec4, 8> aabb_;

	std::vector<rmlv::vec3> position_;
	std::vector<rmlv::vec3> normal_;
	std::vector<rmlv::vec2> texture_;
	std::vector<rmlv::vec3> smoothNormal_;
	std::vector<Face> face_;

	std::vector<Material> material_;

	std::string name_;

	bool solid_;
	std::string message_;

	void Print() const;
	void ComputeBBox();
	void ComputeFaceAndVertexNormals();
	void ComputeEdges(); };


/*inline bool almost_equal(const vao_vertex& a, const vao_vertex& b) {
	return (
		almost_equal(a.point, b.point) &&
		almost_equal(a.normal, b.normal) &&
		almost_equal(a.texcoord, b.texcoord)); }*/


}  // close package namespace
}  // close enterprise namespace
