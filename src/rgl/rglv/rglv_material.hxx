#pragma once
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <optional>
#include <string>

namespace rqdq {
namespace rglv {

struct Material {
	void print() const;
	rmlv::vec3 ka;
	rmlv::vec3 kd;
	rmlv::vec3 ks;
	float specpow;
	float d;
	int pass;
	int invert;
	std::string name;
	std::string imagename;
	std::string shader;
	};


struct MaterialStore {
	void print() const;
	std::optional<int> find_by_name(const std::string& name) const;

	const Material& get(const int id) const {
		return d_store[id]; }

	const int size() const {
		return int(d_store.size()); }

	template <typename func>
	void for_each(const func& f) const {
		for (const auto& item : d_store) {
			f(item); }}

	void append(const Material& m) {
		d_store.push_back(m); }

private:
	rcls::vector<Material> d_store;
	};

}  // close package namespace
}  // close enterprise namespace
