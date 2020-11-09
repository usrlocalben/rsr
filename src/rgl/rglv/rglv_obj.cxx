#include "src/rgl/rglv/rglv_obj.hxx"

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
// #include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <cmath>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace rqdq {

using rmlv::vec4;
using rmlv::vec3;
using rmlv::vec2;
// using rglv::MaterialStore;
using rglv::Material;

using MaterialList = std::vector<Material>;


namespace {

struct SVec3 {

	static
	auto read(std::stringstream& ss) -> SVec3 {
		float x, y, z;
		ss >> x >> y >> z;
		return SVec3{x, y, z}; }

	float x{0}, y{0}, z{0};

	SVec3()  {}
	SVec3(float a) :x(a), y(a), z(a) {}
	SVec3(float x, float y, float z) :x(x), y(y), z(z) {}

	auto xyz1() const -> vec4 {
		return vec4{x, y, z, 1}; }
	auto xyz0() const -> vec4 {
		return vec4{x, y, z, 0}; }
	auto xyz() const -> vec3 {
		return vec3{x, y, z}; }};


struct SVec2 {

	static
	auto read(std::stringstream& ss) -> SVec2 {
		float x, y;
		ss >> x >> y;
		return { x, y }; }

	float x{0}, y{0};

	SVec2()  {}
	SVec2(float x, float y) :x(x), y(y) {}

	auto xy00() const -> vec4 {
		// the bilinear texture sampler
		// assumes that the texcoords
		// will be non-negative. this
		// offset prevents badness for
		// some meshes. XXX
		return { x, y, 0, 0 }; }
		//return rqlma::Vec4{ x, y, 0, 0 }; };

	auto xy() const -> vec2 {
		// the bilinear texture sampler
		// assumes that the texcoords
		// will be non-negative. this
		// offset prevents badness for
		// some meshes. XXX
		return { x, y }; } };


struct faceidx {
	int vv, vt, vn; };


auto to_faceidx(const std::string& data) -> faceidx {
	auto tmp = rclt::Split(data, '/'); // "nn/nn/nn" or "nn//nn", 1+ indexed!!
	return faceidx{
		tmp[0].length() != 0u ? std::stol(tmp[0]) - 1 : 0, // vv
		tmp[1].length() != 0u ? std::stol(tmp[1]) - 1 : 0, // vt
		tmp[2].length() != 0u ? std::stol(tmp[2]) - 1 : 0  // vn
	}; }


struct ObjMaterial {
	std::string name;
	std::string texture;
	SVec3 ka;
	SVec3 kd;
	SVec3 ks;
	float specpow;
	float density;

	void Reset() {
		name = "__none__";
		texture = "";
		ka = SVec3{0.0F};
		kd = SVec3{0.0F};
		ks = SVec3{0.0F};
		specpow = 1.0F;
		density = 1.0F; }

	auto ToMaterial() const -> Material {
		Material mm;
		mm.ka = ka.xyz();
		mm.kd = kd.xyz();
		mm.ks = ks.xyz();
		mm.specpow = specpow;
		mm.d = density;
		mm.name = name;
		mm.imagename = texture;
		mm.pass = 0; // assume pass 0 by default
		return mm; }};


auto LoadMaterials(const std::string& fn) -> MaterialList {

	MaterialList mlst;
	ObjMaterial m;

	auto push = [&mlst, &m]() {
		float p = 1.0f; // 2.2f;
		m.kd.x = pow(m.kd.x, p);
		m.kd.y = pow(m.kd.y, p);
		m.kd.z = pow(m.kd.z, p);
		mlst.push_back(m.ToMaterial()); };

	auto lines = rcls::LoadLines(fn);

	m.Reset();
	for (auto& line : lines) {

		// remove comments, trim, skip empty lines
		auto tmp = line.substr(0, line.find('#'));
		tmp = rclt::Trim(tmp);
		if (tmp.empty()) {
			continue; }

		// create a stream
		std::stringstream ss(tmp, std::stringstream::in);

		std::string cmd;
		ss >> cmd;
		if (cmd == "newmtl") {  //name
			if (m.name != "__none__") {
				push();
				m.Reset(); }
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


}  // namespace

namespace rglv {

auto LoadOBJ(const std::string& fn, std::optional<const std::string> dir) -> Mesh {

	// std::cout << "---- loading [" << fn << "]:" << std::endl;

	auto lines = rcls::LoadLines(fn);
	const auto materialDir = dir.value_or(rcls::DirName(fn));

	std::string group_name("defaultgroup");
	int material_idx = -1;

	Mesh mesh;

	mesh.name_ = fn;

	for (auto& line : lines) {

		auto tmp = line.substr(0, line.find('#'));
		tmp = rclt::Trim(tmp);
		if (tmp.empty()) {
			continue; }
		std::stringstream ss(tmp, std::stringstream::in);

		std::string cmd;
		ss >> cmd;

		if (cmd == "mtllib") {
			// material library
			std::string mtlfn;
			ss >> mtlfn;
			// std::cout << "material library is [" << mtlfn << "]\n";
			mesh.material_ = LoadMaterials(rcls::JoinPath(materialDir, mtlfn)); }

		else if (cmd == "g") {
			// group, XXX unused
			ss >> group_name; }

		else if (cmd == "usemtl") {
			// material setting
			std::string material_name;
			ss >> material_name;
			auto found = std::find_if(begin(mesh.material_), end(mesh.material_), [&](const auto& item) { return item.name == material_name; });
			if (found == end(mesh.material_)) {
				std::cout << "usemtl \"" << material_name << "\" not found in mtl\n";
				throw std::runtime_error("usemtl not found"); }
			material_idx = static_cast<int>(std::distance(begin(mesh.material_), found)); }

		else if (cmd == "v") {
			// vertex
			mesh.position_.push_back(SVec3::read(ss).xyz()); }

		else if (cmd == "vn") {
			// vertex normal
			mesh.normal_.push_back(SVec3::read(ss).xyz()); }

		else if (cmd == "vt") {
			// vertex uv
			mesh.texture_.push_back(SVec2::read(ss).xy()); }

		else if (cmd == "f") {
			// face indices
			std::string data = line.substr(line.find(' ') + 1, std::string::npos);
			auto faces = rclt::Split(data, ' ');
			std::vector<faceidx> indexes;
			indexes.reserve(faces.size());
			for (auto& facechunk : faces) {
				indexes.push_back(to_faceidx(facechunk)); }
			// triangulate and make faces
			for (int i = 0; i < int(indexes.size()) - 2; ++i) {
				Face fd;
				fd.frontMaterial = material_idx;
				fd.idx.position = { { indexes[0].vv, indexes[i+1].vv, indexes[i+2].vv } };
				fd.idx.normal   = { { indexes[0].vn, indexes[i+1].vn, indexes[i+2].vn } };
				fd.idx.texture  = { { indexes[0].vt, indexes[i+1].vt, indexes[i+2].vt } };
				mesh.face_.push_back(fd); }}}

	mesh.ComputeBBox();
	mesh.ComputeFaceAndVertexNormals();
	mesh.ComputeEdges();
	return mesh; }


}  // namespace rglv
}  // namespace rqdq
