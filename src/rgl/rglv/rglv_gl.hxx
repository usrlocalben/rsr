#pragma once
#include <rglr_canvas.hxx>
#include <rglv_gpu_protocol.hxx>
#include <rglv_packed_stream.hxx>
#include <rglv_vao.hxx>
#include <rmlm_mat4.hxx>
#include <rmlv_vec.hxx>

#include <array>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <PixelToaster.h>

namespace rqdq {

namespace {
}  // close unnamed namespace

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

constexpr int GL_MODELVIEW = 1;
constexpr int GL_PROJECTION = 2;
constexpr int GL_TEXTURE = 3;

constexpr int GL_UNSIGNED_SHORT = 1;

constexpr int GL_NEAREST_MIPMAP_NEAREST = 0;
constexpr int GL_LINEAR_MIPMAP_NEAREST = 1;


struct GLState {
	rmlm::mat4 modelViewMatrix;	// defaults to identity
	rmlm::mat4 projectionMatrix;
	rmlm::mat4 textureMatrix;
	bool cullingEnabled;		// default for GL_CULL_FACE is false
	int cullFace;			// default GL_BACK
	rmlv::vec4 color;		// default 1,1,1,1
	rmlv::vec3 normal;      // XXX what's the default?
	int programId;			// default is zero??? unclear
	const void *array;		// default is nullptr
	int arrayFormat;		// unused... 0

	const PixelToaster::FloatingPointPixel *texture0Ptr;
	int texture0Width;
	int texture0Height;
	int texture0Stride;
	int texture0MinFilter;

	void reset() {
		cullingEnabled = false;
		cullFace = GL_BACK;
		color = rmlv::vec4{1.0f,1.0f,1.0f,1.0f};
		normal = rmlv::vec3{ 0.0f, 1.0f, 0.0f };
		programId = 0;

		texture0Ptr = nullptr;
		texture0Width = 8;
		texture0Height = 8;
		texture0Stride = 8;
		texture0MinFilter = GL_NEAREST_MIPMAP_NEAREST;  // standard default is NEAREST_MIPMAP_LINEAR

		array = nullptr;
		arrayFormat = 0; } };


class GL {
public:
	GL() :d_activeMatrixStack(&d_modelViewMatrixStack), d_dirty(false) {}

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

	void glColor(const rmlv::vec3& v) {
		d_dirty = true;
		d_cs.color = rmlv::vec4{ v.x, v.y, v.z, 1.0f }; }

	void glColor(const rmlv::vec4& v) {
		d_dirty = true;
		d_cs.color = v; }

	void glColor(const float r, const float g, const float b) {
		d_dirty = true;
		d_cs.color = rmlv::vec4{ r, g, b, 1.0f }; }

	void glNormal(const rmlv::vec3& v) {
		d_dirty = true;
		d_cs.normal = v; }

	void glUseProgram(const int v) {
		d_dirty = true;
		d_cs.programId = v; }

	void glBindTexture(const PixelToaster::FloatingPointPixel *ptr, int width, int height, int stride, const int mode) {
		d_dirty = true;
		d_cs.texture0Ptr = ptr;
		assert(mode == GL_LINEAR_MIPMAP_NEAREST || mode == GL_NEAREST_MIPMAP_NEAREST);
		d_cs.texture0MinFilter = mode;
		d_cs.texture0Width = width;
		d_cs.texture0Height = height;
		d_cs.texture0Stride = stride; }

	void glUseArray(const VertexArray_F3F3F3& vao) {
		d_dirty = true;
		d_cs.array = static_cast<const void*>(&vao);
		d_cs.arrayFormat = AF_VAO_F3F3F3; }

	//inline void glUniform(const VertexInputUniform& viu) {
	//	state.vertex_input_uniform = viu; }

	//void drawElements(const VertexArray_F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3&);
	void glDrawElements(const int mode, const int count, const int type, const uint16_t * indices, const bool enableClipping=true);
	void glDrawArrays(const int mode, const int start, const int count, const bool enableClipping=true);
	void glClear(const rmlv::vec4 color);
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
	bool d_dirty;
	std::deque<GLState> d_states; };

}  // close package namespace
}  // close enterprise namespace
