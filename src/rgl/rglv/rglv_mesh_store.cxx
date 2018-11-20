#include <rglv_mesh_store.hxx>

#include <rcls_file.hxx>
#include <rglr_texture.hxx>
#include <rglv_material.hxx>
#include <rglv_obj.hxx>
#include <rglv_mesh.hxx>

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <unordered_map>


namespace rqdq {

namespace {
template <typename T>
int len(const T& container) {
	return int(container.size());
}
}

namespace rglv {

using std::cout;
using std::endl;


void MeshStore::load_dir(const std::string& prefix,
						 rglv::MaterialStore& materialstore,
						 rglr::TextureStore& texturestore) {

	const std::string spec = "*.obj";

	for (auto& fn : rcls::fileglob(prefix + spec)) {
		// cout << "loading mesh [" << prefix << "][" << fn << "]" << endl;
		auto [tmp_mesh, tmp_materials] = rglv::loadOBJ(prefix, fn);
		int material_base_idx = materialstore.size();

		tmp_materials.for_each([&](const Material& item) {
			if (item.imagename != "") {
				texturestore.load_any(prefix, item.imagename);
			}
			materialstore.append(item); });

		for (auto& item : tmp_mesh.faces) {
			item.front_material += material_base_idx; }

		store.push_back(tmp_mesh); }}

}  // close package namespace
}  // close enterprise namespace
