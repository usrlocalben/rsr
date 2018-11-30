#pragma once
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"

#include <string>
#include <tuple>
#include <array>


namespace rqdq {
namespace rglv {


class MeshStore {
public:
	const Mesh& get(const std::string& name) const {
		for (const auto& mesh : store) {
			if (mesh.name == name) {
				return mesh; }}
		throw std::exception("mesh not found"); }

	void print() const;
	void load_dir(const std::string& prepend, rglv::MaterialStore& materialstore, rglr::TextureStore& texturestore);

private:
	rcls::vector<Mesh> store;
	};


}  // close package namespace
}  // close enterprise namespace
