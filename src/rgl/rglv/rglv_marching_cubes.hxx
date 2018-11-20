#pragma once
#include <rglv_gpu.hxx>
#include <rglv_vao.hxx>
#include <rmlv_vec.hxx>

#include <array>
#include <mutex>
#include <functional>

namespace rqdq {
namespace rglv {

extern const std::array<rmlv::vec3, 8> vertex_offset;
extern const std::array<rmlv::vec3, 12> edge_direction;
extern const std::array<short, 256> cube_edge_flags;
extern const std::array<std::array<char, 2>, 12> edge_connection;
extern const std::array<std::array<char, 16>, 256> tritable;

/*
 * marching cube cell data
 *
 * order must match vertex_offset table:
 *
 * ccw, starting from left/bottom/back:
 * left/bottom/back, right/bottom/back, right/top/back, left/top/back
 *
 * then again for the front.
 */
struct Cell {
	float value[8];
	rmlv::vec3 pos[8];
	//rmlv::vec3 normal[8];
	};

extern std::mutex strangeMutex;

/*
 * comparitors for surface detection
 */
static const std::less<float> less;
static const std::greater<float> greater;


inline float calc_offset(const float a, const float b, const float target) {
	if (1) {
		return (target - a) / (b - a); }
	else {
		const auto delta = b - a;
		if (delta == 0.0) {
			return 0.5f; }
		else {
			return (target - a) / delta; }}}


/*
 * march one cell, append triangles to SoA vertex array
 *
 * Args:
 *     vao: SoAVertexArray that is ready to append triangle data
 *     pos: position of lower, back, left point of the cell data
 *     delta: distance between grid points
 *     cd: cube/cell data low/left/back ccw, then low/left/front ccw
 *     cmp: surface detection comparison functor
 *     target: surface detection comparison threshold
 */
template <typename FIELD>
void march_sdf_vao(
	VertexArray_PNM& vao,
	const rmlv::vec3 pos,
	const float delta,
	const Cell cd,
	const FIELD& field
	) {
	using rmlv::vec3;
	rmlv::vec3 edge_vertex[12];
	rmlv::vec3 edge_normal[12];

	const rmlv::vec3 vdelta{ delta, delta, delta };

	int flag_index = 0;
	for (int iv = 0; iv<8; iv++) {
		if (cd.value[iv] < 0.0f) {
			flag_index |= 1 << iv; }}

	auto edge_flags = cube_edge_flags[flag_index];

	if (!edge_flags) return;

	const float ep = delta; // 0.001f;

	for (int edge = 0; edge<12; edge++) {
		if (edge_flags & (1 << edge)) {
			auto offset = calc_offset(cd.value[edge_connection[edge][0]],
			                          cd.value[edge_connection[edge][1]],
			                          0);
			auto position = lerp(cd.pos[edge_connection[edge][0]], cd.pos[edge_connection[edge][1]], offset);
			//auto position = pos + ((vertex_offset[edge_connection[edge][0]] + rmlv::vec3{ offset } * edge_direction[edge]) * vdelta);
			edge_vertex[edge] = position;
			float dx = field.sample(position - vec3{ ep,0,0 }) - field.sample(position + vec3{ ep,0,0 });
			float dy = field.sample(position - vec3{ 0,ep,0 }) - field.sample(position + vec3{ 0,ep,0 });
			float dz = field.sample(position - vec3{ 0,0,ep }) - field.sample(position + vec3{ 0,0,ep });
			edge_normal[edge] = normalize(vec3{ dx,dy,dz });}}

	for (int tri = 0; tri < 5; tri++) {
		if (tritable[flag_index][3 * tri] < 0) {
			break; } // end of the list
		for (int corner = 2; corner >= 0; corner--) {
			auto vertex = tritable[flag_index][3 * tri + corner];
			vao.append(edge_vertex[vertex], edge_normal[vertex], vec3{0}); }}}


}  // close package namespace
}  // close enterprise namespace
