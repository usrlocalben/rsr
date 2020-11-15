#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglr/rglr_texture_load.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_texture.hxx"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace rqdq;
using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl final : public ITexture {
	const rglr::Texture texture_;

public:
	Impl(std::string_view id, InputList inputs, rglr::Texture texture) :
		ITexture(id, std::move(inputs)),
		texture_(std::move(texture)) {}

	// -- ITexture --
	auto GetTexture() -> const rglr::Texture& override {
		return texture_; }};


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		rglr::Texture tex;
		if (auto jv = jv_find(data_, "file", JSON_STRING)) {
			tex = rglr::LoadPNG(jv->toString(), jv->toString(), false);
			tex.maybe_make_mipmap(); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), tex); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$image", [](){ return std::make_unique<Compiler>(); });
}} init{};

}  // close unnamed namespace
