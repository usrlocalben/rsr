#include <array>
#include <cmath>
#include <iostream>
#include <string>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rgl/rglr/rglr_algorithm.hxx"
#include "src/rgl/rglr/rglr_blend.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_interpolate.hxx"
#include "src/rgl/rglv/rglv_triangle.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

using namespace std;
using namespace rqdq;
using vec2 = rmlv::vec2;
using vec3 = rmlv::vec3;
using vec4 = rmlv::vec4;
using qfloat4 = rmlv::qfloat4;
using qfloat2 = rmlv::qfloat2;
using VertexFloat1 = rglv::VertexFloat1;
using VertexFloat2 = rglv::VertexFloat2;
using VertexFloat3 = rglv::VertexFloat3;

const int TARGET_WIDTH = 8;
const int TARGET_HEIGHT = 8;

bool check_triangle_uv();
bool check_triangle(array<vec4, 3> /*points*/, array<string, 8> /*expected*/);

template <typename SHADER_PROGRAM, typename BLEND_PROGRAM>
struct TestTargetProgram {
	rmlv::qfloat4* cb_;
	rmlv::qfloat* db_;

	const int stride_;
	int offs_, offsLeft_;

	const rglv::VertexFloat1 oneOverW_;
	const rglv::VertexFloat1 zOverW_;

	const typename SHADER_PROGRAM::Interpolants vo_;

	TestTargetProgram(
		rglr::QFloat4Canvas& cc,
		rglr::QFloatCanvas& dc,
		VertexFloat1 oneOverW,
		VertexFloat1 zOverW,
		typename SHADER_PROGRAM::VertexOutputSD computed0,
		typename SHADER_PROGRAM::VertexOutputSD computed1,
		typename SHADER_PROGRAM::VertexOutputSD computed2) :
		cb_(cc.data()),
		db_(dc.data()),
		stride_(cc.width() >> 1),
		oneOverW_(oneOverW),
		zOverW_(zOverW),
		vo_(computed0, computed1, computed2) {}

	inline void Begin(int x, int y) {
		offs_ = offsLeft_ = (y>>1)*stride_ + (x>>1); }

	inline void CR() {
		offsLeft_ += stride_;
		offs_ = offsLeft_; }

	inline void Right2() {
		offs_++; }

	inline void LoadDepth(rmlv::qfloat& destDepth) {
		// from depthbuffer
		// destDepth = _mm_load_ps(reinterpret_cast<float*>(&db[offs]));

		// from alpha
		destDepth = _mm_load_ps(reinterpret_cast<float*>(&(cb_[offs_].a))); }

	inline void StoreDepth(rmlv::qfloat destDepth,
	                       rmlv::qfloat sourceDepth,
	                       rmlv::mvec4i fragMask) {
		// auto addr = &db_[offs_];   // depthbuffer
		auto addr = &(cb_[offs_].a);  // alpha-channel
		auto result = SelectFloat(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }

	inline void LoadColor(rmlv::qfloat4& destColor) {
		rglr::QFloat4Canvas::Load(cb_+offs_, destColor.x.v, destColor.y.v, destColor.z.v); }

	inline void StoreColor(rmlv::qfloat4 destColor,
	                       rmlv::qfloat4 sourceColor,
	                       rmlv::mvec4i fragMask) {
		auto sr = SelectFloat(destColor.r, sourceColor.r, fragMask).v;
		auto sg = SelectFloat(destColor.g, sourceColor.g, fragMask).v;
		auto sb = SelectFloat(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat4Canvas::Store(sr, sg, sb, cb_+offs_); }

	inline void Render(const rmlv::qfloat2 fragCoord, const rmlv::mvec4i triMask, rglv::BaryCoord BS) {
		using rmlv::qfloat, rmlv::qfloat3, rmlv::qfloat4;

		const auto fragDepth = rglv::Interpolate(BS, zOverW_);

		// read depth buffer
		qfloat destDepth; LoadDepth(destDepth);

		const auto depthMask = float2bits(cmple(fragDepth, destDepth));
		const auto fragMask = andnot(triMask, depthMask);

		if (movemask(bits2float(fragMask)) == 0) {
			return; }  // early out if whole quad fails depth test

		// restore perspective
		const auto fragW = rmlv::oneover(Interpolate(BS, oneOverW_));
		rglv::BaryCoord BP;
		BP.x = oneOverW_.v0 * BS.x * fragW;
		BP.z = oneOverW_.v2 * BS.z * fragW;
		BP.y = 1.0f - BP.x - BP.z;

		auto data = vo_.Interpolate(BS, BP);

		qfloat4 fragColor;
		SHADER_PROGRAM::ShadeFragment(fragCoord, fragDepth, data, BS, BP, fragColor);

		qfloat4 destColor;
		LoadColor(destColor);

		// qfloat4 blendedColor = fragColor; // no blending

		StoreColor(destColor, fragColor, fragMask);
		StoreDepth(destDepth, fragDepth, fragMask); } };


struct TestShaderBase {
	struct VertexOutputSD {
		rmlv::vec3 r0;
		static VertexOutputSD Mix(VertexOutputSD a, VertexOutputSD b, float t) {
			return { mix(a.r0, b.r0, t) }; }};

	struct VertexOutputMD {
		rmlv::qfloat3 r0; };

	struct Interpolants {
		Interpolants(VertexOutputSD d0, VertexOutputSD d1, VertexOutputSD d2) :
			r0({ d0.r0, d1.r0, d2.r0 }) {}
		VertexOutputMD Interpolate(const rglv::BaryCoord& BS, const rglv::BaryCoord& BP) const {
			return { rglv::Interpolate(BP, r0) }; }
		rglv::VertexFloat3 r0; };

	inline static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// vertex shader output
		const VertexOutputMD& v,
		// special
		const rglv::BaryCoord& BS, const rglv::BaryCoord& BP,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = rmlv::mvec4f{1.0F}; }};


struct DebugWithFragCoord final : public TestShaderBase {
	static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// vertex shader output
		const VertexOutputMD& v,
		// special
		const rglv::BaryCoord& BS, const rglv::BaryCoord& BP,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = qfloat4{ gl_FragCoord.x, gl_FragCoord.y, 0, 0 }; } };


struct DebugWithBary final : public TestShaderBase {
	static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// vertex shader output
		const VertexOutputMD& v,
		// special
		const rglv::BaryCoord& BS, const rglv::BaryCoord& BP,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = qfloat4{ BS.x, BS.y, BS.z, 0 }; } };


struct DebugWithUV : public TestShaderBase {
	static void ShadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// vertex shader output
		const VertexOutputMD& v,
		// special
		const rglv::BaryCoord& BS, const rglv::BaryCoord& BP,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
			gl_FragColor = qfloat4{ v.r0.x, v.r0.y, 0, 0 }; }};


int main(int argc, char **argv) {
	int total_tests{ 0 }, total_errors{ 0 };

	// test dx reference triangle
	{
		total_tests++;
		string test_name{ "dx top-left fill rules #1" };

		array<vec4, 3> points = {
			vec4{ 2.0F, 4.0F, 0, 1 },
			vec4{ 6.0F, 2.0F, 0, 1 },
			vec4{ 1.0F, 1.0F, 0, 1 },
			};

		array<string, 8> expected = {
			"........",
			".XX.....",
			".XXXX...",
			"..X.....",
			"........",
			"........",
			"........",
			"........"
			};

		bool ok = check_triangle(points, expected);
		if (!ok) {
			cout << test_name << ": failed" << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; } }

	{
		total_tests++;
		string test_name{ "2560x2560 uv interpoliation with integers on pixel centers" };
		bool ok = check_triangle_uv();
		if (!ok) {
			cout << test_name << ": failed" << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; } }

	cout << endl;
	return total_errors != 0 ? 1 : 0; }


bool check_triangle(array<vec4, 3> points, array<string, 8> expected) {

	/*
	vec4 dp1{ 0.5f, 0.5f, 0,1 };
	vec4 dp2{ 0.5f, 2.5f, 0,1 };
	vec4 dp3{ 2.5f, 0.5f, 0,1 };

	vec4 dp1{ 2.5f, 0.5f, 0,1 };
	vec4 dp2{ 0.5f, 2.5f, 0,1 };
	vec4 dp3{ 2.5f, 2.5f, 0, 1 };
	*/

	rglr::QFloat4Canvas cbc(8, 8);
	rglr::QFloatCanvas dbc(8, 8);

	rglr::Fill(cbc, vec4{ 0, 0, 0, 1.0F });

	TestTargetProgram<DebugWithBary, rglr::BlendProgram::Set> target_program(
		cbc, dbc,
		VertexFloat1{ points[0].w, points[1].w, points[2].w },
		VertexFloat1{ points[0].z, points[1].z, points[2].z },
		DebugWithBary::VertexOutputSD{},
		DebugWithBary::VertexOutputSD{},
		DebugWithBary::VertexOutputSD{});
	rglv::TriangleRasterizer<false, decltype(target_program)> tr(target_program, cbc.rect(), cbc.height());
	tr.Draw(points[0], points[1], points[2]);

	// compare
	for (int yy = 0; yy < 8; yy++) {
		//std::cout << "\n";
		auto cmprow = expected[yy];
		for (int xx = 0; xx < 8; xx++) {
			auto px = cbc.get_pixel(xx, yy);
			auto cmpch = px.x > 0 ? 'X' : '.';
			//std::cout <<cmpch;
			if (cmpch != cmprow[xx]) {
				return false; } }}
	return true; }


/**
 * cover a large target with UV coords corresponding to the target x,y
 * verify that the target is filled with the correct u,v values within 0.01f
 */
bool check_triangle_uv() {
#if 0
	const int width = 256;
	const int height = 256;
	const float fwidth = 256.0f;
	const float fheight = 256.0f;
	const float right = 256.5f;
	const float bottom = 256.5f;
#else
	const int dim = 2560;
	const int w = dim;
	const int h = dim;
#endif

	rglr::QFloat4Canvas cbc(w, h);
	rglr::QFloatCanvas dbc(w, h);

	rglr::Fill(cbc, vec4{ 0, 0, 0, 1.0F });

	vec4 ulp( 0, 0, 0, 1 ); vec4 urp( w, 0, 0, 1 );
	vec4 llp( 0, h, 0, 1 ); vec4 lrp( w, h, 0, 1 );

	vec2 uluv( 0, h ); vec2 uruv( w, h );
	vec2 lluv( 0, 0 ); vec2 lruv( w, 0 );

	array<vec4, 3> points_upper_left = { ulp, llp, urp, };
	array<vec2, 3> uv_upper_left = { uluv, lluv, uruv, };
	array<vec4, 3> points_lower_right = { urp, llp, lrp };
	array<vec2, 3> uv_lower_right = { uruv, lluv, lruv };

	{
		auto& points = points_upper_left;
		auto& uvs = uv_upper_left;
		DebugWithUV::VertexOutputSD computed1; computed1.r0 = vec3{ uvs[0], 0 };
		DebugWithUV::VertexOutputSD computed2; computed2.r0 = vec3{ uvs[1], 0 };
		DebugWithUV::VertexOutputSD computed3; computed3.r0 = vec3{ uvs[2], 0 };
		TestTargetProgram<DebugWithUV, rglr::BlendProgram::Set> target_program(
			cbc, dbc,
			VertexFloat1{ points[0].w, points[1].w, points[2].w },
			VertexFloat1{ points[0].z, points[1].z, points[2].z },
			computed1, computed2, computed3);
		rglv::TriangleRasterizer<false, decltype(target_program)> tr(target_program, cbc.rect(), cbc.height());
		tr.Draw(points[0], points[1], points[2]); }
	{
		auto& points = points_lower_right;
		auto& uvs = uv_lower_right;
		DebugWithUV::VertexOutputSD computed1; computed1.r0 = vec3{ uvs[0], 0 };
		DebugWithUV::VertexOutputSD computed2; computed2.r0 = vec3{ uvs[1], 0 };
		DebugWithUV::VertexOutputSD computed3; computed3.r0 = vec3{ uvs[2], 0 };
		TestTargetProgram<DebugWithUV, rglr::BlendProgram::Set> target_program(
			cbc, dbc,
			VertexFloat1{ points[0].w, points[1].w, points[2].w },
			VertexFloat1{ points[0].z, points[1].z, points[2].z },
			computed1, computed2, computed3);
		rglv::TriangleRasterizer<false, decltype(target_program)> tr(target_program, cbc.rect(), cbc.height());
		tr.Draw(points[0], points[1], points[2]); }

	auto almostEqual = [](float a, float b) {
		constexpr float ep = 0.01F;
		return (b-ep < a && a < b+ep); };


	// cout << "=========== data ============\n";
	for (int yy = 0; yy < h; yy++) {
		for (int xx = 0; xx < w; xx++) {
			auto px = cbc.get_pixel(xx, yy);

			// expected values
			auto ex =   xx   + 0.5F;
			auto ey = (h-yy) - 0.5F;

			// cerr << "xx(" << xx << ") yy(" << yy << ") px(" << px.x << ", " << px.y << ") expected(" << ex << ", " << ey << ")\n";

			if (!almostEqual(px.x, ex)) {
				// verify U is [0.5 1.5, 2.5, ... w-0.5] left to right
				return false; }
			if (!almostEqual(px.y, ey)) {
				// verify V is [h-0.5, ... 2.5, 1.5, 0.5] top to bottom
				return false; }
		}
		//cerr << "\n";
	}
	return true; }

		/*
		auto bcheck = Barycentric(
			vec4{ 2.5f, 3.5f, 0, 0 },
			dp1, dp2, dp3
		);
		cout << bcheck <<"\n";
vec4 Barycentric(vec4 p, vec4 a, vec4 b, vec4 c) {
	float u, v, w;

	vec4 v0 = b - a, v1 = c - a, v2 = p - a;
	float den = v0.x * v1.y - v1.x * v0.y;
	v = (v2.x * v1.y - v1.x * v2.y) / den;
	w = (v0.x * v2.y - v2.x * v0.y) / den;
	u = 1.0f - v - w;

	return { u,v,w,0 };
}
		*/
