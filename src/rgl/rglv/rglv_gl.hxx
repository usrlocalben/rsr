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
constexpr int GL_SCISSOR_TEST = 2;

constexpr int GL_FRONT = 1;
constexpr int GL_BACK = 2;
constexpr int GL_FRONT_AND_BACK = (GL_FRONT | GL_BACK);

constexpr int GL_UNSIGNED_SHORT = 1;

constexpr int GL_NEAREST_MIPMAP_NEAREST = 0;
constexpr int GL_LINEAR_MIPMAP_NEAREST = 1;

constexpr int GL_COLOR_BUFFER_BIT = 1;
constexpr int GL_DEPTH_BUFFER_BIT = 2;
constexpr int GL_STENCIL_BUFFER_BIT = 4;

constexpr uint8_t RGL_HINT_READ4 = 1;
constexpr uint8_t RGL_HINT_DENSE = 2;

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
	bool scissorEnabled;
	rmlv::ivec2 scissorOrigin;
	rmlv::ivec2 scissorSize;
	rmlv::ivec2 viewportOrigin;
	std::optional<rmlv::ivec2> viewportSize;

	int programId;			// default is zero??? unclear
	int uniformsOfs;

	std::array<const void*, 4> buffers;
	std::array<int, 4> bufferFormat;

	std::array<TextureState, 2> tus;

	const PixelToaster::FloatingPointPixel *texture1Ptr;
	int texture1Width;
	int texture1Height;
	int texture1Stride;
	int texture1MinFilter;

	rmlm::mat4 viewMatrix;
	rmlm::mat4 normalMatrix;
	rmlm::mat4 projectionMatrix;

	auto VertexStateKey() const -> uint32_t {
		uint32_t key = programId << 24;
		return key; }

	auto FragmentStateKey() const -> uint32_t {
		uint32_t key = programId << 24;
		key |= scissorEnabled;
		return key; }

	auto BltStateKey() const -> uint32_t {
		uint32_t key = programId << 24;
		return key; }

	void reset() {
		clearColor = rmlv::vec4{ 0.0F, 0.0F, 0.0F, 1.0F };
		clearDepth = 1.0F;
		clearStencil = 0;

		cullingEnabled = false;
		cullFace = GL_BACK;

		scissorEnabled = false;

		viewportOrigin = rmlv::ivec2{ 0, 0 };
		viewportSize = std::nullopt;

		programId = 0;
		uniformsOfs = -1;

		for (auto& tu : tus) {
			tu.reset(); }
		for (auto& item : buffers) {
			item = nullptr; }
		for (auto& item : bufferFormat) {
			item = 0; }}};


class GL {
public:
	GL() = default;

	void Enable(const int value) {
		dirty_ = true;
		if (value == GL_CULL_FACE) {
			cs_.cullingEnabled = true; }
		else if (value == GL_SCISSOR_TEST) {
			cs_.scissorEnabled = true; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void Disable(const int value) {
		dirty_ = true;
		if (value == GL_CULL_FACE) {
			cs_.cullingEnabled = false; }
		else if (value == GL_SCISSOR_TEST) {
			cs_.scissorEnabled = false; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void CullFace(const int value) {
		dirty_ = true;
		cs_.cullFace = value; }

	void Scissor(int x, int y, int width, int height) {
		dirty_ = true;
		cs_.scissorOrigin = rmlv::ivec2{ x, y };
		cs_.scissorSize = rmlv::ivec2{ width, height }; }

	void Viewport(int x, int y, int width, int height) {
		dirty_ = true;
		cs_.viewportOrigin = rmlv::ivec2{ x, y };
		cs_.viewportSize = rmlv::ivec2{ width, height }; }

	void UseProgram(const int v) {
		dirty_ = true;
		cs_.programId = v; }

	void UseUniforms(const int ofs) {
		dirty_ = true;
		cs_.uniformsOfs = ofs; }

	template <typename T>
	std::pair<int, T*> AllocUniformBuffer() {
		const int amt = (sizeof(T) + 0xf) & 0xfffffff0;
		int idx = ubuf_.size();
		for (int i=0; i<amt; i++) { ubuf_.emplace_back(); }
		auto* ptr = reinterpret_cast<T*>(ubuf_.data() + idx);
		return { idx, ptr }; }

	void* GetUniformBufferAddr(int ofs) {
		if (ofs == -1) {
			return nullptr; }
		return &ubuf_[ofs]; }

	void BindTexture(int unit, const PixelToaster::FloatingPointPixel *ptr, int width, int height, int stride, const int mode) {
		dirty_ = true;
		assert(mode == GL_LINEAR_MIPMAP_NEAREST || mode == GL_NEAREST_MIPMAP_NEAREST);
		auto& tu = cs_.tus[unit];
		tu.ptr = ptr;
		tu.filter = mode;
		tu.width = width;
		tu.height = height;
		tu.stride = stride; }

	void UseBuffer(int idx, const VertexArray_F3F3F3& vao) {
		dirty_ = true;
		cs_.buffers[idx] = &vao;
		cs_.bufferFormat[idx] = AF_VAO_F3F3F3; }

	void UseBuffer(int idx, float* ptr) {
		dirty_ = true;
		cs_.buffers[idx] = ptr;
		cs_.bufferFormat[idx] = AF_FLOAT; }

	void ClearColor(rmlv::vec3 value) {
		dirty_ = true;
		cs_.clearColor = rmlv::vec4{ value, 1.0F }; }

	void ClearDepth(float value) {
		dirty_ = true;
		cs_.clearDepth = value; }

	void ViewMatrix(const rmlm::mat4& m) {
		dirty_ = true;
		cs_.viewMatrix = m; }

	void ProjectionMatrix(const rmlm::mat4& m) {
		dirty_ = true;
		cs_.projectionMatrix = m; }

	void NormalMatrix(const rmlm::mat4& m) {
		dirty_ = true;
		cs_.normalMatrix = m; }

	//inline void glUniform(const VertexInputUniform& viu) {
	//	state.vertex_input_uniform = viu; }

	//void drawElements(const VertexArray_F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3&);
	void DrawElements(int mode, int count, int type, const uint16_t* indices, uint8_t hint=0);
	void DrawArrays(int mode, int start, int count);
	void DrawElementsInstanced(int mode, int count, int type, const uint16_t* indices, int instanceCnt);
	void DrawArraysInstanced(int mode, int start, int count, int instanceCnt);
	void Clear(int bits);
	void StoreColor(rglr::FloatingPointCanvas *dst, bool downsample);
	void StoreColor(rglr::QFloat4Canvas *dst);
	void StoreColor(rglr::TrueColorCanvas *dst, bool enableGammaCorrection);
	// void StoreDepth(rglr::QFloatCanvas *dst);
	void Finish() {}
	void Reset();

private:
	void MaybeUpdateState();

public:
	mutable std::mutex mutex;
	FastPackedStream commands_;

private:
	GLState cs_;
	bool dirty_{false};
	std::vector<uint8_t> ubuf_;
	std::deque<GLState> states_; };


}  // namespace rglv
}  // namespace rqdq
