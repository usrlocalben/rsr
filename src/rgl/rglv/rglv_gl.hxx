#pragma once
#include <array>
#include <deque>
#include <mutex>
#include <stdexcept>

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include "3rdparty/pixeltoaster/PixelToaster.h"

namespace rqdq {
namespace rglv {

constexpr int GL_TRIANGLES = 0;
constexpr int GL_QUADS = 1;
constexpr int GL_TRIANGLE_STRIP = 2;
constexpr int GL_QUAD_STRIP = 3;

constexpr int GL_CW = 0;
constexpr int GL_CCW = 1;

constexpr int GL_CULL_FACE = 1;

constexpr int GL_FRONT = 1;
constexpr int GL_BACK = 2;
constexpr int GL_FRONT_AND_BACK = (GL_FRONT | GL_BACK);

constexpr int GL_UNSIGNED_SHORT = 1;

constexpr int GL_NEAREST_MIPMAP_NEAREST = 0;
constexpr int GL_LINEAR_MIPMAP_NEAREST = 1;

constexpr int GL_COLOR_BUFFER_BIT = 1;
constexpr int GL_DEPTH_BUFFER_BIT = 2;
constexpr int GL_STENCIL_BUFFER_BIT = 4;

struct TextureState {
	const PixelToaster::FloatingPointPixel *ptr;
	int width;
	int height;
	int stride;
	int filter;

	void reset() {
		ptr = nullptr;
		width = 8;
		height = 8;
		stride = 8;
		filter = GL_NEAREST_MIPMAP_NEAREST;  // standard default is NEAREST_MIPMAP_LINEAR
		} };


struct GLState {
	rmlv::vec4 clearColor;
	float clearDepth;
	int clearStencil;

	bool cullingEnabled;		// default for GL_CULL_FACE is false
	int cullFace;			// default GL_BACK

	int programId;			// default is zero??? unclear
	int uniformsOfs;

	const void *array;		// default is nullptr
	int arrayFormat;		// unused... 0

	std::array<TextureState, 2> tus;

	const PixelToaster::FloatingPointPixel *texture1Ptr;
	int texture1Width;
	int texture1Height;
	int texture1Stride;
	int texture1MinFilter;

	void reset() {
		clearColor = rmlv::vec4{ 0.0F, 0.0F, 0.0F, 1.0F };
		clearDepth = 1.0F;
		clearStencil = 0;

		cullingEnabled = false;
		cullFace = GL_BACK;

		programId = 0;
		uniformsOfs = -1;

		for (auto& tu : tus) {
			tu.reset(); }
		array = nullptr;
		arrayFormat = 0; } };


class GL {
public:
	GL() = default;

	void glEnable(const int value) {
		d_dirty = true;
		if (value == GL_CULL_FACE) {
			d_cs.cullingEnabled = true; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void glDisable(const int value) {
		d_dirty = true;
		if (value == GL_CULL_FACE) {
			d_cs.cullingEnabled = false; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void glCullFace(const int value) {
		d_dirty = true;
		d_cs.cullFace = value; }

	void glUseProgram(const int v) {
		d_dirty = true;
		d_cs.programId = v; }

	void glUniforms(const int ofs) {
		d_dirty = true;
		d_cs.uniformsOfs = ofs; }

	template <typename T>
	std::pair<int, T*> glReserveUniformBuffer() {
		const int amt = (sizeof(T) + 0xf) & 0xfffffff0;
		int idx = d_ubuf.size();
		for (int i=0; i<amt; i++) { d_ubuf.emplace_back(); }
		auto* ptr = reinterpret_cast<T*>(d_ubuf.data() + idx);
		return { idx, ptr }; }

	void* glGetUniformBufferAddr(int ofs) {
		return &d_ubuf[ofs]; }

	void glBindTexture(int unit, const PixelToaster::FloatingPointPixel *ptr, int width, int height, int stride, const int mode) {
		d_dirty = true;
		assert(mode == GL_LINEAR_MIPMAP_NEAREST || mode == GL_NEAREST_MIPMAP_NEAREST);
		auto& tu = d_cs.tus[unit];
		tu.ptr = ptr;
		tu.filter = mode;
		tu.width = width;
		tu.height = height;
		tu.stride = stride; }

	void glUseArray(const VertexArray_F3F3F3& vao) {
		d_dirty = true;
		d_cs.array = static_cast<const void*>(&vao);
		d_cs.arrayFormat = AF_VAO_F3F3F3; }

	void glClearColor(rmlv::vec3 value) {
		d_dirty = true;
		d_cs.clearColor = rmlv::vec4{ value, 1.0F }; }

	void glClearDepth(float value) {
		d_dirty = true;
		d_cs.clearDepth = value; }

	//inline void glUniform(const VertexInputUniform& viu) {
	//	state.vertex_input_uniform = viu; }

	//void drawElements(const VertexArray_F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3&);
	void glDrawElements(int mode, int count, int type, const uint16_t* indices);
	void glDrawArrays(int mode, int start, int count);
	void glClear(int bits);
	void storeHalfsize(rglr::FloatingPointCanvas *dst);
	void storeUnswizzled(rglr::FloatingPointCanvas *dst);
	void storeTrueColor(bool enableGammaCorrection, rglr::TrueColorCanvas* dst);
	void endDrawing() {}
	void reset();

private:
	void maybeUpdateState();

public:
	mutable std::mutex mutex;
	FastPackedStream d_commands;

private:
	GLState d_cs;
	bool d_dirty{false};
	std::vector<uint8_t> d_ubuf;
	std::deque<GLState> d_states; };


}  // namespace rglv
}  // namespace rqdq
