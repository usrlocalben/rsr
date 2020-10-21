#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_mesh_util.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/fontloader.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"
#include "src/viewer/node/i_material.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;

struct FontInfo {
	int height;
	int ascenderHeight;
	int mapDim;
	std::unordered_map<char, GlyphInfo> glyphs; };


class Impl final : public IGl {
	// config
	const FontInfo font_;
	const float wrapWidth_;
	const float leading_;
	const float tracking_;

	// inputs
	IMaterial* materialNode_{nullptr};
	IValue* textNode_{nullptr};
	std::string textSlot_{"default"};
	IValue* colorNode_{nullptr};
	std::string colorSlot_{"default"};

	// runtime
	int mod2_{0};
	std::array<rglv::VertexArray_F3F3F3, 2> vbos_;

public:
	Impl(std::string_view id, InputList inputs, FontInfo fi, float wrapWidth, float leading, float tracking) :
		IGl(id, std::move(inputs)),
		font_(std::move(fi)),
		wrapWidth_(wrapWidth),
		leading_(leading),
		tracking_(tracking) {}

	auto Connect(std::string_view attr, NodeBase* other, std::string_view slot) -> bool override {
		if (attr == "material") {
			materialNode_ = dynamic_cast<IMaterial*>(other);
			if (materialNode_ == nullptr) {
				TYPE_ERROR(IMaterial);
				return false; }
			return true; }
		if (attr == "text") {
			textNode_ = dynamic_cast<IValue*>(other);
			textSlot_ = slot;
			if (textNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		if (attr == "color") {
			colorNode_ = dynamic_cast<IValue*>(other);
			colorSlot_ = slot;
			if (colorNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return IGl::Connect(attr, other, slot); }

	void Main() override {
		auto postSetup = Compute();
		AddLinksTo(postSetup);
		materialNode_->AddLink(postSetup);
		materialNode_->Run(); }

private:
	rclmt::jobsys::Job* Compute(rclmt::jobsys::Job* parent = nullptr) {
		if (parent != nullptr) {
			return rclmt::jobsys::make_job_as_child(parent, Impl::ComputeJmp, std::tuple{this}); }
		return rclmt::jobsys::make_job(Impl::ComputeJmp, std::tuple{this}); }
	static void ComputeJmp(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<Impl*>* data) {
		auto&[self] = *data;
		self->ComputeImpl(); }
	void ComputeImpl() {
		mod2_ = (mod2_+1)%2;
		auto& vbo = vbos_[mod2_];
		vbo.clear();

		rmlv::vec3 color{1.0F};
		if (colorNode_ != nullptr) {
			color = colorNode_->Eval(colorSlot_).as_vec3(); }

		auto text = textNode_->Eval(textSlot_).as_string();
		auto words = rclt::Split(text, ' ');
		std::vector<std::string> lines(1);
		int li{0};
		for (const auto& word : words) {
			auto tmp = rclt::Trim(lines[li] + " " + word);
			if (TextWidth(tmp) >= wrapWidth_) {
				lines.emplace_back(word);
				++li; }
			else {
				lines[li] = tmp; }}

		// for (const auto& line : lines) {
		//	std::cerr << "l: \"" << line << "\"\n"; }

		float cy = -1;
		for (const auto& line : lines) {
			float cx = 0;
			for (auto ch : line) {
				GlyphInfo glyph;
				if (auto found = font_.glyphs.find(ch); found != end(font_.glyphs)) {
					glyph = found->second; }
				else {
					glyph = font_.glyphs.find(' ')->second; }
				const auto dim = static_cast<float>(font_.mapDim);
				const int width=glyph.width, x=glyph.x, y=glyph.y, w=glyph.w, h=glyph.h, ox=glyph.ox, oy=glyph.oy;
				const int depth = font_.height - font_.ascenderHeight;

				float u0 = float(x+0) / dim;
				float v0 = 1.0F - float(y+0) / dim;
				float u1 = float(x+w) / dim;
				float v1 = 1.0F - float(y+h+0) / dim;

				float x0 = cx + (glyph.ox / float(font_.height));
				float y0 = cy + ((glyph.oy+depth) / float(font_.height));
				float x1 = x0 + (glyph.w / float(font_.height));
				float y1 = y0 - (glyph.h / float(font_.height));

				cx += (width - ox) / float(font_.height) * tracking_;

				// auto n = rmlv::vec3{ 0, 0, 1.0F };
				vbo.append({ x0, y0, 0 }, color, { u0, v0, 0 });
				vbo.append({ x0, y1, 0 }, color, { u0, v1, 0 });
				vbo.append({ x1, y0, 0 }, color, { u1, v0, 0 });

				vbo.append({ x1, y0, 0 }, color, { u1, v0, 0 });
				vbo.append({ x0, y1, 0 }, color, { u0, v1, 0 });
				vbo.append({ x1, y1, 0 }, color, { u1, v1, 0 }); }
			cy -= 1.0F * leading_; }

		vbo.pad(); }

	void AddDeps() override {
		AddDep(materialNode_); }

	void DrawDepth(rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		std::lock_guard lock(dc.mutex);

		dc.ViewMatrix(*mvmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, vbos_[mod2_]);
		dc.DrawArrays(GL_TRIANGLES, 0, vbos_[mod2_].size()); }

	void Draw(int pass, const LightPack& lights [[maybe_unused]], rglv::GL* _dc, const rmlm::mat4* pmat, const rmlm::mat4* vmat, const rmlm::mat4* mmat) override {
		using namespace rglv;
		auto& dc = *_dc;
		if (pass != 1) return;
		std::lock_guard lock(dc.mutex);
		if (materialNode_ != nullptr) {
			materialNode_->Apply(_dc); }

		dc.ViewMatrix(*vmat * *mmat);
		dc.ProjectionMatrix(*pmat);

		dc.UseBuffer(0, vbos_[mod2_]);
		dc.DrawArrays(GL_TRIANGLES, 0, vbos_[mod2_].size()); }

private:
	auto TextWidth(const std::string& text) -> float {
		float cx = 0;
		float ax = 0;
		for (auto ch : text) {
			GlyphInfo glyph;
			if (auto found = font_.glyphs.find(ch); found != end(font_.glyphs)) {
				glyph = found->second; }
			else {
				glyph = font_.glyphs.find(' ')->second; }
			const int width=glyph.width, ox=glyph.ox;
			ax = cx + (width - ox) / float(font_.height);
			cx += (width - ox) / float(font_.height) * tracking_; }
		return ax; }};


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("material", /*required=*/true)) { return; }
		if (!Input("text", /*required=*/true)) { return; }
		if (!Input("color", /*required=*/false)) { return; }

		std::string_view font{"notfound.lua"};
		if (auto jv = rclx::jv_find(data_, "font", JSON_STRING)) {
			font = jv->toString(); }

		float width = 10.0F;
		float leading = 1.0F;
		float tracking = 1.0F;
		if (auto jv = rclx::jv_find(data_, "width", JSON_NUMBER)) {
			width = static_cast<float>(jv->toNumber()); }
		if (auto jv = rclx::jv_find(data_, "leading", JSON_NUMBER)) {
			leading = static_cast<float>(jv->toNumber()); }
		if (auto jv = rclx::jv_find(data_, "tracking", JSON_NUMBER)) {
			tracking = static_cast<float>(jv->toNumber()); }

		LuaFontLoader lfl(font.data());

		FontInfo fi;
		fi.height = lfl.HeightInPx();
		fi.ascenderHeight = lfl.AscenderHeightInPx();
		fi.mapDim = lfl.TextureDim();
		for (int i=0, sz=lfl.Length(); i<sz; ++i) {
			auto gi = lfl.Entry(i);
			fi.glyphs[gi.ch] = gi; }

		out_ = std::make_shared<Impl>(id_, std::move(inputs_), std::move(fi), width, leading, tracking); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$writer", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
