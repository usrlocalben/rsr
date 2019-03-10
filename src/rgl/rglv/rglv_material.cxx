#include "src/rgl/rglv/rglv_material.hxx"

#include <iostream>
#include <optional>

namespace rqdq {
namespace rglv {


void MaterialStore::print() const {
	std::cout << "----- material pack -----" << std::endl;
	for (int i = 0; i < d_store.size(); i++) {
		auto& item = d_store[i];
		std::cout << "id = " << i << ", ";
		item.print();
	}
	std::cout << "---  end of materials ---" << std::endl; }


std::optional<int> MaterialStore::find_by_name(const std::string& name) const {
	for (int i = 0; i<d_store.size(); i++) {
		if (d_store[i].name == name)
			return i; }
	return {}; }


void Material::print() const {
	std::cout << "material[" << this->name << "]:" << std::endl;
	std::cout << "  ka" << this->ka << ", ";
	std::cout << "kd" << this->kd << ", ";
	std::cout << "ks" << this->ks << std::endl;
	std::cout << "  specpow(" << this->specpow << "), density(" << this->d << ")" << std::endl;
	std::cout << "  texture[" << this->imagename << "]" << std::endl; }


}  // namespace rglv
}  // namespace rqdq
