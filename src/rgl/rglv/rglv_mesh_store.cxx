#include "src/rgl/rglv/rglv_mesh_store.hxx"

#include "src/rcl/rcls/rcls_file.hxx"
#include "src/rgl/rglv/rglv_obj.hxx"

#include <algorithm>
#include <memory_resource>
#include <string>
#include <string_view>

#include <fmt/format.h>

namespace rqdq {
namespace {

template <typename T1, typename T2>
auto EndsWithI(const T1& suffix, const T2& text) -> bool {
	return equal(rbegin(suffix), rend(suffix), rbegin(text),
	             [](char a, char b) { return tolower(a) == tolower(b); }); }


}  // close unnamed namespace

namespace rglv {


void MeshStore::LoadDir(std::string_view dir) {
	char buf[4096];
	std::pmr::monotonic_buffer_resource pool(buf, sizeof(buf));
	auto lst = rcls::ListDir(dir, &pool);

	std::pmr::string path(&pool);
	path.reserve(dir.size() + 32);
	for (const auto& item : lst) {
		rcls::JoinPath(dir, item, path);
		if (EndsWithI(std::string_view{".obj"}, item)) {
			fmt::print("loading mesh \"{}\"\n", item);
			auto mesh = rglv::LoadOBJ(path);
			store_.push_back(mesh); }}}


}  // close package namespace
}  // close enterprise namespace
