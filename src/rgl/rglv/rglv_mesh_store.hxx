#pragma once
#include <string_view>
#include <tuple>
#include <array>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"

namespace rqdq {
namespace rglv {

class MeshStore {
	std::vector<Mesh> store_;

public:
	auto get(std::string_view name) const -> const Mesh& {
		for (const auto& mesh : store_) {
			if (mesh.name_ == name) {
				return mesh; }}
		throw std::exception("mesh not found"); }

	void Print() const;
	void LoadDir(const std::string& dir); };


}  // namespace rglv
}  // namespace rqdq
