#include "src/rgl/rglv/rglv_mesh_store.hxx"

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_obj.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"

namespace rqdq {
namespace {

template <typename T>
int len(const T& container) {
	return int(container.size()); }


}  // namespace

namespace rglv {

using std::cout;
using std::endl;

void MeshStore::LoadDir(const std::string& dir) {

	const std::string spec = "*.obj";

	for (auto& fn : rcls::FindGlob(rcls::JoinPath(dir, spec))) {
		// cout << "loading mesh [" << prefix << "][" << fn << "]" << endl;
		auto mesh = rglv::LoadOBJ(rcls::JoinPath(dir, fn));
		store_.push_back(mesh); }}


}  // namespace rglv
}  // namespace rqdq
