#include <rglv_obj.hxx>
#include <rcls_file.hxx>
#include <rclt_util.hxx>
#include <rglv_material.hxx>
#include <rglv_mesh.hxx>
#include <rmlv_vec.hxx>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace rqdq {

using rmlv::vec4;
using rmlv::vec3;
using rmlv::vec2;
using rglv::MaterialStore;
using rglv::Material;

namespace {

struct SVec3 {
	float x, y, z;
	SVec3() :x(0), y(0), z(0) {}
	SVec3(float a) :x(a), y(a), z(a) {}
	SVec3(float x, float y, float z) :x(x), y(y), z(z) {}

	vec4 xyz1() const {
		return vec4{x, y, z, 1}; }
	vec4 xyz0() const {
		return vec4{x, y, z, 0}; }
	vec3 xyz() const {
		return vec3{x, y, z}; }
	static SVec3 read(std::stringstream& ss) {
		float x, y, z;
		ss >> x >> y >> z;
		return SVec3{x, y, z}; }
	};


struct SVec2 {
	float x, y;
	SVec2() :x(0), y(0) {}
	SVec2(float x, float y) :x(x), y(y) {}
	vec4 xy00() const {
		// the bilinear texture sampler
		// assumes that the texcoords
		// will be non-negative. this
		// offset prevents badness for
		// some meshes. XXX
		return vec4{ x+100.0f, y+100.0f, 0, 0 }; }
		//return rqlma::Vec4{ x, y, 0, 0 }; };
	vec2 xy() const {
		// the bilinear texture sampler
		// assumes that the texcoords
		// will be non-negative. this
		// offset prevents badness for
		// some meshes. XXX
		return vec2{ x + 100.0f, y + 100.0f }; }
		//return rqlma::Vec4{ x, y, 0, 0 }; };
	static SVec2 read(std::stringstream& ss) {
		float x, y;
		ss >> x >> y;
		return SVec2{x, y}; }
	};


struct faceidx {
	int vv, vt, vn; };


faceidx to_faceidx(const std::string& data) {
	auto tmp = rclt::explode(data, '/'); // "nn/nn/nn" or "nn//nn", 1+ indexed!!
	return faceidx{
		tmp[0].length() ? std::stol(tmp[0]) - 1 : 0, // vv
		tmp[1].length() ? std::stol(tmp[1]) - 1 : 0, // vt
		tmp[2].length() ? std::stol(tmp[2]) - 1 : 0  // vn
	}; }


struct ObjMaterial {
	std::string name;
	std::string texture;
	SVec3 ka;
	SVec3 kd;
	SVec3 ks;
	float specpow;
	float density;

	void reset() {
		name = "__none__";
		texture = "";
		ka = SVec3{0.0f};
		kd = SVec3{0.0f};
		ks = SVec3{0.0f};
		specpow = 1.0f;
		density = 1.0f; }

	Material to_material() const {
		Material mm;
		mm.ka = ka.xyz();
		mm.kd = kd.xyz();
		mm.ks = ks.xyz();
		mm.specpow = specpow;
		mm.d = density;
		mm.name = "obj-" + name;
		mm.imagename = texture;
		mm.shader = "obj";
		mm.pass = 0; // assume pass 0 by default
		return mm; }};


MaterialStore loadMaterials(const std::string& fn) {

	MaterialStore mlst;
	ObjMaterial m;

	auto push = [&mlst,&m]() {
		float p = 1.0f; // 2.2f;
		m.kd.x = pow(m.kd.x, p);
		m.kd.y = pow(m.kd.y, p);
		m.kd.z = pow(m.kd.z, p);
		mlst.append(m.to_material()); };

	auto lines = rcls::loadFileAsLines(fn);

	m.reset();
	for (auto& line : lines) {

		// remove comments, trim, skip empty lines
		auto tmp = line.substr(0, line.find('#'));
		tmp = rclt::trim(tmp);
		if (tmp.size() == 0) continue;

		// create a stream
		std::stringstream ss(tmp, std::stringstream::in);

		std::string cmd;
		ss >> cmd;
		if (cmd == "newmtl") {  //name
			if (m.name != "__none__") {
				push();
				m.reset(); }
			ss >> m.name; }
		else if (cmd == "Ka") { // ambient color
			m.ka = SVec3::read(ss); }
		else if (cmd == "Kd") { // diffuse color
			m.kd = SVec3::read(ss); }
		else if (cmd == "Ks") { // specular color
			m.ks = SVec3::read(ss); }
		else if (cmd == "Ns") { // phong exponent
			ss >> m.specpow; }
		else if (cmd == "d") { // opacity ("d-issolve")
			ss >> m.density; }
		else if (cmd == "map_Kd") { //diffuse texturemap
			ss >> m.texture; }}

	if (m.name != "__none__") {
		push(); }

	return mlst; }
}  // close unnamed namespace

namespace rglv {

std::tuple<Mesh,MaterialStore> loadOBJ(const std::string& prepend, const std::string& fn) {

	// std::cout << "---- loading [" << fn << "]:" << std::endl;

	auto lines = rcls::loadFileAsLines(prepend + fn);

	std::string group_name("defaultgroup");
	int material_idx = -1;

	MaterialStore materials;
	Mesh mesh;

	mesh.name = fn;

	for (auto& line : lines) {

		auto tmp = line.substr(0, line.find('#'));
		tmp = rclt::trim(tmp);
		if (tmp.size() == 0) continue;
		std::stringstream ss(tmp, std::stringstream::in);

		std::string cmd;
		ss >> cmd;

		if (cmd == "mtllib") {
			// material library
			std::string mtlfn;
			ss >> mtlfn;
			// std::cout << "material library is [" << mtlfn << "]\n";
			materials = loadMaterials(prepend + mtlfn); }

		else if (cmd == "g") {
			// group, XXX unused
			ss >> group_name; }

		else if (cmd == "usemtl") {
			// material setting
			std::string material_name;
			ss >> material_name;
			auto found = materials.find_by_name(std::string("obj-") + material_name);
			if (!found.has_value()) {
				std::cout << "usemtl \"" << material_name << "\" not found in mtl\n";
				throw std::runtime_error("usemtl not found"); }
			else {
				material_idx = found.value(); }}

		else if (cmd == "v") {
			// vertex
			mesh.points.push_back(SVec3::read(ss).xyz()); }

		else if (cmd == "vn") {
			// vertex normal
			mesh.normals.push_back(SVec3::read(ss).xyz()); }

		else if (cmd == "vt") {
			// vertex uv
			mesh.texcoords.push_back(SVec2::read(ss).xy()); }

		else if (cmd == "f") {
			// face indices
			std::string data = line.substr(line.find(' ') + 1, std::string::npos);
			auto faces = rclt::explode(data, ' ');
			std::vector<faceidx> indexes;
			for (auto& facechunk : faces) {
				indexes.push_back(to_faceidx(facechunk)); }
			// triangulate and make faces
			for (int i = 0; i < indexes.size() - 2; i++){
				Face fd;
				fd.front_material = material_idx;
				fd.point_idx = { { indexes[0].vv, indexes[i + 1].vv, indexes[i + 2].vv } };
				fd.normal_idx = { { indexes[0].vn, indexes[i + 1].vn, indexes[i + 2].vn } };
				fd.texcoord_idx = { { indexes[0].vt, indexes[i + 1].vt, indexes[i + 2].vt } };
				mesh.faces.push_back(fd); }}}

	mesh.compute_bbox();
	mesh.compute_face_and_vertex_normals();
	mesh.compute_edges();
	return std::tuple<Mesh, MaterialStore>(mesh, materials); }

}  // close package namespace
}  // close enterprise namespace
