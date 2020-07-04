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

void MeshStore::LoadDir(const std::string& prefix) {

	const std::string spec = "*.obj";

	for (auto& fn : rcls::FindGlob(prefix + spec)) {
		// cout << "loading mesh [" << prefix << "][" << fn << "]" << endl;
		auto tmp_mesh = rglv::LoadOBJ(prefix, fn);
		store_.push_back(tmp_mesh); }}


}  // namespace rglv
}  // namespace rqdq
