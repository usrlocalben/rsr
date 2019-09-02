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
using VertexFloat2 = rglv::VertexFloat2;

const int TARGET_WIDTH = 8;
const int TARGET_HEIGHT = 8;

bool check_triangle_uv();
bool check_triangle(array<vec4, 3> /*points*/, array<string, 8> /*expected*/);

struct DebugWithFragCoord final : public rglv::BaseProgram {
	template <typename TU>
	static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _BS, const rglv::BaryCoord& _BP,
		// texture units
		const TU& tu0, const TU& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = qfloat4{ gl_FragCoord.x, gl_FragCoord.y, 0, 0 }; } };


struct DebugWithBary final : public rglv::BaseProgram {
	template <typename TU>
	static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _BS, const rglv::BaryCoord& _BP,
		// texture units
		const TU& tu0, const TU& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
		gl_FragColor = qfloat4{ _BS.x, _BS.y, _BS.z, 0 }; } };


struct DebugWithUV : public rglv::BaseProgram {
	template <typename TU>
	static void shadeFragment(
		// built-in
		const rmlv::qfloat2& gl_FragCoord, /* gl_FrontFacing, */ const rmlv::qfloat& gl_FragDepth,
		// unforms
		const rglv::ShaderUniforms& u,
		// vertex shader output
		const rglv::VertexOutput& outs,
		// special
		const rglv::BaryCoord& _BS, const rglv::BaryCoord& _BP,
		// texture units
		const TU& tu0,
		const TU& tu1,
		// outputs
		rmlv::qfloat4& gl_FragColor
		) {
			gl_FragColor = qfloat4{ outs.r0.x, outs.r0.y, 0, 0 }; }};


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

	rglr::Fill(cbc, vec4{ 0, 0, 0, 1.0F }, cbc.rect());

	bool frontfacing = true;

	using sampler = rglr::ts_pow2_mipmap;
	const sampler nullTexture1(nullptr, 256, 256, 256, 0);
	const sampler nullTexture2(nullptr, 256, 256, 256, 0);
	rglv::ShaderUniforms nullUniforms{};
	rglv::DefaultTargetProgram<sampler, DebugWithBary, rglr::BlendProgram::Set> target_program(
		nullTexture1, nullTexture2, cbc, dbc,
		nullUniforms,
		points[0], points[1], points[2],
		rglv::VertexOutputx1{}, rglv::VertexOutputx1{}, rglv::VertexOutputx1{});
	rglv::TriangleRasterizer tr(target_program, cbc.rect(), cbc.height());
	tr.Draw(points[0], points[1], points[2], frontfacing);

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
	/*const int width = 2560;
	const int height = 2560;
	const float fwidth = 2560.0f;
	const float fheight = 2560.0f;
	const float right = 2560.5f;
	const float bottom = 2560.5f;*/
	const int width = 16;
	const int height = 16;
	const float fwidth = 16.0F;
	const float fheight = 16.0F;
	const float right = 16.5F;
	const float bottom = 16.5F;

	rglr::QFloat4Canvas cbc(width, height);
	rglr::QFloatCanvas dbc(width, height);

	rglr::Fill(cbc, vec4{ 0, 0, 0, 1.0F }, cbc.rect());

	bool frontfacing = true;

	vec4 ulp{ 0.5, 0.5, 0, 1 };     vec2 uluv{ 0.0, fheight };  vec4 urp{ right,    0.5, 0, 1 };  vec2 uruv{ fwidth, fheight };
	vec4 llp{ 0.5, bottom, 0, 1 };  vec2 lluv{ 0.0,    0.0 };  vec4 lrp{ right, bottom, 0, 1 };  vec2 lruv{ fwidth,   0.0 };

	array<vec4, 3> points_upper_left = { ulp, llp, urp, };
	array<vec2, 3> uv_upper_left = { uluv, lluv, uruv, };
	array<vec4, 3> points_lower_right = { urp, llp, lrp };
	array<vec2, 3> uv_lower_right = { uruv, lluv, lruv };

	using sampler = rglr::ts_pow2_mipmap;
	const sampler nullTexture1(nullptr, 256, 256, 256, 0);
	const sampler nullTexture2(nullptr, 256, 256, 256, 0);
	rglv::ShaderUniforms nullUniforms{};

	{
		auto& points = points_upper_left;
		auto& uvs = uv_upper_left;
		rglv::VertexOutputx1 computed1; computed1.r0 = vec3{ uvs[0], 0 };
		rglv::VertexOutputx1 computed2; computed2.r0 = vec3{ uvs[1], 0 };
		rglv::VertexOutputx1 computed3; computed3.r0 = vec3{ uvs[2], 0 };
		rglv::DefaultTargetProgram<sampler, DebugWithUV, rglr::BlendProgram::Set> target_program(
			nullTexture1, nullTexture2, cbc, dbc,
			nullUniforms,
			points[0], points[1], points[2],
			computed1, computed2, computed3);
		rglv::TriangleRasterizer tr(target_program, cbc.rect(), cbc.height());
		tr.Draw(points[0], points[1], points[2], frontfacing); }
	{
		auto& points = points_lower_right;
		auto& uvs = uv_lower_right;
		rglv::VertexOutputx1 computed1; computed1.r0 = vec3{ uvs[0], 0 };
		rglv::VertexOutputx1 computed2; computed2.r0 = vec3{ uvs[1], 0 };
		rglv::VertexOutputx1 computed3; computed3.r0 = vec3{ uvs[2], 0 };
		rglv::DefaultTargetProgram<sampler, DebugWithUV, rglr::BlendProgram::Set> target_program(
			nullTexture1, nullTexture2, cbc, dbc,
			nullUniforms,
			points[0], points[1], points[2],
			computed1, computed2, computed3);
		rglv::TriangleRasterizer tr(target_program, cbc.rect(), cbc.height());
		tr.Draw(points[0], points[1], points[2], frontfacing); }


	// cout << "=========== data ============\n";
	const float expected_accuracy = 0.01F;
	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			auto px = cbc.get_pixel(xx, yy);

			int iu = int(px.x + expected_accuracy);
			int iv = int(px.y + expected_accuracy);

			//float dx = float(xx) - px.x;
			//float dy = float(height - yy) - px.y;
			//cout << "(" << dx << "," << dy << ") ";
			// cout << "(" << int(px.x+0.01f) << "," << int(px.y+0.01f) << ") ";
			if (iu != xx) {
				// verify U is 0,1,2,3,4,5,6,7 from lef to right
				return false; }
			if (iv != (height - yy)) {
				// verify V is 8,7,6,5,4,3,2,1 from top to bottom
				return false; }
		}
		//cout << "\n";
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
