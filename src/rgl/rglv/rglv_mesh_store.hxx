#pragma once
#include <rcls_aligned_containers.hxx>
#include <rglr_texture.hxx>
#include <rglv_material.hxx>
#include <rglv_mesh.hxx>

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
