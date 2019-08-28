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
constexpr int GL_SCISSOR = 2;

constexpr int GL_FRONT = 1;
constexpr int GL_BACK = 2;
constexpr int GL_FRONT_AND_BACK = (GL_FRONT | GL_BACK);

constexpr int GL_MODELVIEW = 1;
constexpr int GL_PROJECTION = 2;
constexpr int GL_TEXTURE = 3;

constexpr int GL_UNSIGNED_SHORT = 1;

constexpr int GL_NEAREST_MIPMAP_NEAREST = 0;
constexpr int GL_LINEAR_MIPMAP_NEAREST = 1;

constexpr int GL_COLOR_BUFFER_BIT = 1;
constexpr int GL_DEPTH_BUFFER_BIT = 2;
// constexpr int GL_STENCIL_BUFFER_BIT = 4;

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
	rmlm::mat4 modelViewMatrix;	// defaults to identity
	rmlm::mat4 projectionMatrix;
	rmlm::mat4 textureMatrix;
	rmlv::vec4 clearColor;
	float clearDepth;
	int clearStencil;
	rmlv::ivec2 viewportOrigin;
	rmlv::ivec2 viewportDim;
	rmlv::ivec2 scissorOrigin;
	rmlv::ivec2 scissorDim;
	bool scissorEnabled;
	bool cullingEnabled;		// default for GL_CULL_FACE is false
	int cullFace;			// default GL_BACK
	rmlv::vec4 color;		// default 1,1,1,1
	rmlv::vec3 normal;      // XXX what's the default?
	int programId;			// default is zero??? unclear
	const void *array;		// default is nullptr
	int arrayFormat;		// unused... 0

	std::array<TextureState, 2> tus;

	const PixelToaster::FloatingPointPixel *texture1Ptr;
	int texture1Width;
	int texture1Height;
	int texture1Stride;
	int texture1MinFilter;

	void reset() {
		clearStencil = 0;
		clearDepth = 1.0F;
		clearColor = rmlv::vec4{ 0.0F, 0.0F, 0.0F, 1.0F };
		viewportOrigin = { 0, 0 };
		viewportDim = { -1, -1 };
		scissorOrigin = { 0, 0 };
		scissorDim = { -1, -1 };
		scissorEnabled = false;
		cullingEnabled = false;
		cullFace = GL_BACK;
		color = rmlv::vec4{1.0F,1.0F,1.0F,1.0F};
		normal = rmlv::vec3{ 0.0F, 1.0F, 0.0F };
		programId = 0;
		for (auto& tu : tus) {
			tu.reset(); }
		array = nullptr;
		arrayFormat = 0; } };


class GL {
public:
	GL() :d_activeMatrixStack(&d_modelViewMatrixStack) {}

	void glPushMatrix() {
		d_dirty = true;
		d_activeMatrixStack->push(); }

	void glPopMatrix() {
		d_dirty = true;
		d_activeMatrixStack->pop(); }

	void glMultMatrix(const rmlm::mat4& m) {
		d_dirty = true;
		d_activeMatrixStack->mul(m); }

	void glTranslate(const rmlv::vec4& a) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::translate(a)); }

	void glTranslate(const rmlv::vec3& a) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::translate(a)); }

	void glTranslate(const float x, const float y, const float z) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::translate(x, y, z)); }

	void glScale(const rmlv::vec4& a) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::scale(a)); }

	void glScale(const float x, const float y, const float z) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::scale(x, y, z)); }

	void glRotate(const float theta, const float x, const float y, const float z) {
		d_dirty = true;
		d_activeMatrixStack->mul(rmlm::mat4::rotate(theta, x, y, z)); }

	void glLoadIdentity() {
		d_dirty = true;
		d_activeMatrixStack->load(rmlm::mat4::ident()); }

	void glLoadMatrix(const rmlm::mat4& m) {
		d_dirty = true;
		d_activeMatrixStack->load(m); }

	void glMatrixMode(const int value) {
		if (value == GL_MODELVIEW) {
			d_activeMatrixStack = &d_modelViewMatrixStack; }
		else if (value == GL_PROJECTION) {
			d_activeMatrixStack = &d_projectionMatrixStack; }
		else if (value == GL_TEXTURE) {
			d_activeMatrixStack = &d_textureMatrixStack; }
		else {
			throw std::runtime_error("unknown matrix mode"); }}

	void glEnable(const int value) {
		d_dirty = true;
		if (value == GL_CULL_FACE) {
			d_cs.cullingEnabled = true; }
		else if (value == GL_SCISSOR) {
			d_cs.scissorEnabled = true; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void glDisable(const int value) {
		d_dirty = true;
		if (value == GL_CULL_FACE) {
			d_cs.cullingEnabled = false; }
		else if (value == GL_SCISSOR) {
			d_cs.scissorEnabled = true; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void glCullFace(const int value) {
		d_dirty = true;
		d_cs.cullFace = value; }

	void glColor(const rmlv::vec3& v) {
		d_dirty = true;
		d_cs.color = rmlv::vec4{ v.x, v.y, v.z, 1.0F }; }

	void glColor(const rmlv::vec4& v) {
		d_dirty = true;
		d_cs.color = v; }

	void glColor(const float r, const float g, const float b) {
		d_dirty = true;
		d_cs.color = rmlv::vec4{ r, g, b, 1.0F }; }

	void glNormal(const rmlv::vec3& v) {
		d_dirty = true;
		d_cs.normal = v; }

	void glUseProgram(const int v) {
		d_dirty = true;
		d_cs.programId = v; }

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

	void glViewport(int x, int y, int width, int height) {
		d_dirty = true;
		d_cs.viewportOrigin = rmlv::ivec2{ x, y };
		d_cs.viewportDim = rmlv::ivec2{ width, height }; }

	void glScissor(int x, int y, int width, int height) {
		d_dirty = true;
		d_cs.scissorOrigin = rmlv::ivec2{ x, y };
		d_cs.scissorDim = rmlv::ivec2{ width, height }; }


	//inline void glUniform(const VertexInputUniform& viu) {
	//	state.vertex_input_uniform = viu; }

	//void drawElements(const VertexArray_F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3&);
	void glDrawElements(int mode, int count, int type, const uint16_t* indices, bool enableClipping=true);
	void glDrawArrays(int mode, int start, int count, bool enableClipping=true);
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
	rmlm::Mat4Stack d_modelViewMatrixStack;
	rmlm::Mat4Stack d_projectionMatrixStack;
	rmlm::Mat4Stack d_textureMatrixStack;
	rmlm::Mat4Stack* d_activeMatrixStack;

	GLState d_cs;
	bool d_dirty{false};
	std::deque<GLState> d_states; };


}  // namespace rglv
}  // namespace rqdq
