#include "src/rgl/rglv/rglv_obj.hxx"

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <charconv>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory_resource>
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

auto SplitWhitespce(std::string_view text, std::pmr::memory_resource* mem) -> std::pmr::vector<std::pmr::string> {
	std::string_view delims(" \t\n\r");
	std::pmr::vector<std::pmr::string> out(mem);
	std::size_t start = text.find_first_not_of(delims), end = 0;
	while ((end = text.find_first_of(delims, start)) != std::pmr::string::npos) {
		out.emplace_back(text.substr(start, end-start));
		start = text.find_first_not_of(delims, end); }
	if (start != std::pmr::string::npos) {
		out.emplace_back(text.substr(start)); }
	return out; }


auto SplitFloats(std::string_view text, int limit, float* out) {
	if (limit <= 0) return;
	int idx{ 0 };
	std::string_view delims(" \t");
	std::size_t start = text.find_first_not_of(delims), end = 0;
	while ((end = text.find_first_of(delims, start)) != std::pmr::string::npos) {
		auto segBegin = text.data() + start;
		// auto segEnd = text.data() + end;
		auto res = std::from_chars(segBegin, text.data() + text.size(), out[idx++], std::chars_format::general);
		if (res.ec != std::errc()) {
			std::printf("bad float [%llu, %llu)\n", start, end);
			std::exit(1); }
		if (idx == limit) return;
		start = text.find_first_not_of(delims, end); }
	if (start != std::pmr::string::npos) {
		auto res = std::from_chars(text.data() + start, text.data() + text.size(), out[idx++], std::chars_format::general);
		if (res.ec != std::errc()) {
			std::printf("bad float [%llu, %llu)\n", start, end);
			std::exit(1); }}}


auto ParseVec3(std::string_view text) -> vec3 {
	float f[3];
	SplitFloats(text, 3, f);
	return { f[0], f[1], f[2] }; }


auto ParseVec2(std::string_view text) -> vec2 {
	float f[2];
	SplitFloats(text, 2, f);
	return { f[0], f[1] }; }


auto ParseFloat(std::string_view text) -> float {
	float f;
	SplitFloats(text, 1, &f);
	return f; }


struct faceidx {
	int vv, vt, vn; };


auto to_faceidx(std::string_view data) -> faceidx {
	char buf[128];
	std::pmr::monotonic_buffer_resource pool(buf, sizeof(buf));

	// "nn/nn/nn" or "nn//nn", 1+ indexed!!
	auto tmp = rclt::Split(data, '/', &pool);

	// default to 1 for e.g. 'nn//nn'
	int idx[3] = { 1, 1, 1 };

	for (int i = 0; i < 3; ++i) {
		if (tmp[i].size()) {
			auto res = std::from_chars(tmp[i].data(), tmp[i].data()+tmp[i].size(), idx[i]);
			if (res.ec != std::errc()) {
				throw std::runtime_error("bad integer in obj face indices");}}}

	return faceidx{ idx[0]-1, idx[1]-1, idx[2]-1 }; } // vv, vt, vn


struct ObjMaterial {
	std::string name;
	std::string texture;
	vec3 ka;
	vec3 kd;
	vec3 ks;
	float specpow;
	float density;

	void Reset() {
		name.assign("__none__");
		texture.clear();
		ka = 0;
		kd = 0;
		ks = 0;
		specpow = 1.F;
		density = 1.F; }

	auto ToMaterial() const -> Material {
		Material mm;
		mm.ka = ka;
		mm.kd = kd;
		mm.ks = ks;
		mm.specpow = specpow;
		mm.d = density;
		mm.name = name;
		mm.imagename = texture;
		mm.pass = 0; // assume pass 0 by default
		return mm; }};


auto ParseMtllib(std::istream& fd) -> MaterialList {
	MaterialList mlst;
	ObjMaterial m;

	auto push = [&mlst, &m]() {
#if 0
		// gamma-space to linear-space
		float p = 2.2F;
		m.kd.x = pow(m.kd.x, p);
		m.kd.y = pow(m.kd.y, p);
		m.kd.z = pow(m.kd.z, p);
#endif
		mlst.push_back(m.ToMaterial()); };

	char buf[256];
	std::pmr::monotonic_buffer_resource pool(buf, sizeof(buf));
	std::pmr::string line(&pool);
	line.reserve(255);

	m.Reset();
	while (getline(fd, line)) {
		auto text = std::string_view(line);
		text = text.substr(0, text.find('#')); // remove end-of-line comments
		text = rclt::TrimView(text); // trim whitespace
		if (text.empty()) { continue; } // skip empty lines
		std::string_view cmd;
		tie(cmd, text) = rclt::Split1View(text); // take the command off the front

		if (cmd == "newmtl") {  //name
			if (m.name != "__none__") {
				push();
				m.Reset(); }
			m.name = text; }
		else if (cmd == "Ka") { // ambient color
			m.ka = ParseVec3(text); }
		else if (cmd == "Kd") { // diffuse color
			m.kd = ParseVec3(text); }
		else if (cmd == "Ks") { // specular color
			m.ks = ParseVec3(text); }
		else if (cmd == "Ns") { // phong exponent
			m.specpow = ParseFloat(text); }
		else if (cmd == "d") { // opacity ("d-issolve")
			m.density = ParseFloat(text); }
		else if (cmd == "map_Kd") { //diffuse texturemap
			m.texture = text; }}

	if (m.name != "__none__") {
		push(); }

	return mlst; }


}  // namespace

namespace rglv {

auto LoadOBJ(const std::pmr::string& fn) -> Mesh {
	char buf[1024];
	std::pmr::monotonic_buffer_resource pool(buf, sizeof(buf));

	std::pmr::string line(&pool);
	line.reserve(100);

	std::ifstream fd(fn.c_str());

	std::pmr::string groupName("defaultgroup", &pool);

	auto objDir = rcls::DirNameView(fn);

	auto [ll, rr] = rcls::SplitPath(fn);
	// const auto materialDir = dir ? dir : rcls::DirName(fn);

	int material_idx = -1;

	Mesh mesh;
	mesh.name_ = rr; // XXX basename

	while (getline(fd, line)) {
		// std::cerr << "LINE: \"" << line << "\"\n";
		auto text = std::string_view(line);
		// std::cerr << "TEXT: \"" << text << "\"\n";
		text = text.substr(0, text.find('#')); // remove end-of-line comments
		// std::cerr << "TEXT: \"" << text << "\"\n";
		text = rclt::TrimView(text); // trim whitespace
		// std::cerr << "TEXT: \"" << text << "\"\n";
		if (text.empty()) { continue; } // skip empty lines
		std::string_view cmd;
		tie(cmd, text) = rclt::Split1View(text); // take the command off the front
		// std::cerr << "COMMAND: \"" << cmd << "\"\n";
		// std::cerr << "TEXT: \"" << text << "\"\n";

		if (cmd == "mtllib") {
			// material library
			auto mtlPath = rcls::JoinPath(objDir, text, &pool);
// std::cerr << "obj: mtllib " << mtlPath << "\n";
			std::ifstream mtlFd(mtlPath.c_str());
			mesh.material_ = ParseMtllib(mtlFd); }

		else if (cmd == "g") {
			// group, XXX unused
			groupName = text; }

		else if (cmd == "usemtl") {
			// material setting
			auto mtlName = text;
			auto found = find_if(begin(mesh.material_), end(mesh.material_),
			                     [&](const auto& item) { return item.name == mtlName; });
			if (found == end(mesh.material_)) {
				std::cerr << "usemtl \"" << mtlName << "\" not found in mtl\n";
				throw std::runtime_error("usemtl not found"); }
			material_idx = static_cast<int>(std::distance(begin(mesh.material_), found)); }

		else if (cmd == "v") {
			// vertex
			mesh.position_.push_back(ParseVec3(text)); }

		else if (cmd == "vn") {
			// vertex normal
			mesh.normal_.push_back(ParseVec3(text)); }

		else if (cmd == "vt") {
			// vertex uv
			mesh.texture_.push_back(ParseVec2(text)); }

		else if (cmd == "f") {
			// face indices
			char faceBuf[512];
			std::pmr::monotonic_buffer_resource facePool(faceBuf, sizeof(faceBuf));
			std::pmr::vector<faceidx> indexes(&facePool);
			auto faces = rclt::Split(text, ' ', &facePool);
			indexes.reserve(faces.size());
			for (auto& facechunk : faces) {
				indexes.push_back(to_faceidx(facechunk)); }
			// triangulate and make faces
			for (int i=0, siz=int(indexes.size()); i<siz-2; ++i) {
				Face f;
				f.frontMaterial = material_idx;
				f.idx.position = { { indexes[0].vv, indexes[i+1].vv, indexes[i+2].vv } };
				f.idx.normal   = { { indexes[0].vn, indexes[i+1].vn, indexes[i+2].vn } };
				f.idx.texture  = { { indexes[0].vt, indexes[i+1].vt, indexes[i+2].vt } };
				mesh.face_.push_back(f); }}}

	mesh.ComputeBBox();
	mesh.ComputeFaceAndVertexNormals();
	mesh.ComputeEdges();
	return mesh; }


}  // namespace rglv
}  // namespace rqdq
