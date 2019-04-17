#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglr/rglr_texture_load.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_texture.hxx"

namespace rqdq {
namespace {

using namespace rqv;

class Impl final : public ITexture {
public:
	Impl(std::string_view id, InputList inputs, rglr::Texture texture)
		:ITexture(id, std::move(inputs)), texture_(std::move(texture)) {}

	const rglr::Texture& GetTexture() override {
		return texture_; };

private:
	rglr::Texture texture_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		rglr::Texture tex;
		if (auto jv = jv_find(data_, "file", JSON_STRING)) {
			tex = rglr::load_png(jv->toString(), jv->toString(), false);
			tex.maybe_make_mipmap(); }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), tex); }};


Compiler compiler{};

struct init { init() {
	NodeRegistry::GetInstance().Register("$image", &compiler);
}} init{};


}  // namespace
}  // namespace rqdq
