#pragma once
#include <array>
#include <cstdint>
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
constexpr int GL_BLEND = 3;
constexpr int GL_DEPTH_TEST = 4;

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

constexpr int GL_LESS = 0;
constexpr int GL_LEQUAL = 1;
constexpr int GL_EQUAL = 2;

constexpr int GL_DEPTH_ATTACHMENT = 0;
constexpr int GL_STENCIL_ATTACHMENT = 1;
constexpr int GL_COLOR_ATTACHMENT0 = 2;

constexpr int RB_COLOR_DEPTH = 0;
constexpr int RB_RGBF32 = 1;
constexpr int RB_RGBAF32 = 2;
constexpr int RB_F32 = 3;

constexpr int UNIFORM_BUFFER_SIZE = 8*4;  // 32 floats, or 8 vec4s

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
	bool blendingEnabled;		// false, disabled
	bool colorWriteMask;		// true, enabled
	bool depthWriteMask;		// true, enabled
	bool depthTestEnabled;      // 
	int depthFunc; 

	int programId;			// default is zero??? unclear
	int uniformsOfs;

	int color0AttachmentType;  // default is RB_COLOR_DEPTH
	int depthAttachmentType;   // default is RB_COLOR_DEPTH

	std::array<const float*, 16> buffers;

	std::array<TextureState, 2> tus;

	const float* tu3ptr;
	int tu3dim;

	rmlm::mat4 viewMatrix;
	rmlm::mat4 normalMatrix;
	rmlm::mat4 projectionMatrix;

	auto VertexStateKey() const -> uint32_t {
		uint32_t key = programId << 24;
		return key; }

	void Dump() const {
		std::cerr << "<GLState pgm=" << programId;
		std::cerr << " A(C0)=" << color0AttachmentType;
		std::cerr << " A(D)=" << depthAttachmentType;
		if (cullingEnabled) std::cerr << " CULL_FACE";
		if (cullFace & GL_BACK) std::cerr << " BACK";
		if (cullFace & GL_FRONT) std::cerr << " FRONT";
		if (scissorEnabled) std::cerr << " SCISSOR_TEST";
		if (depthTestEnabled) std::cerr << " DEPTH_TEST";
		if (depthFunc == GL_LESS) std::cerr << " LESS";
		if (depthFunc == GL_LEQUAL) std::cerr << " LEQUAL";
		if (depthFunc == GL_EQUAL) std::cerr << " EQUAL";
		if (blendingEnabled) std::cerr << " BLEND";
		if (depthWriteMask) std::cerr << " DEPTH_WRITE";
		if (colorWriteMask) std::cerr << " COLOR_WRITE";
		std::cerr << ">\n"; }

	auto FragmentStateKey() const -> uint32_t {
		uint32_t key = programId << 24;
		key |= static_cast<uint32_t>(scissorEnabled);
		key |= depthTestEnabled<<1;
		if (depthTestEnabled) key |= depthFunc<<2;
		key |= blendingEnabled<<4;
		key |= depthWriteMask<<5;
		key |= colorWriteMask<<6;
		key |= color0AttachmentType<<7;
		key |= depthAttachmentType<<9;
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
		blendingEnabled = false;
		colorWriteMask = true;
		depthWriteMask = true;
		depthTestEnabled = true;
		depthFunc = GL_LESS;

		viewportOrigin = rmlv::ivec2{ 0, 0 };
		viewportSize = std::nullopt;

		programId = 0;
		uniformsOfs = -1;

		tu3ptr = nullptr;
		tu3dim = 256;

		for (auto& tu : tus) {
			tu.reset(); }
		for (auto& item : buffers) {
			item = nullptr; }}};


class GL {
public:
	GL() = default;

	void Enable(const int value) {
		dirty_ = true;
		if (value == GL_CULL_FACE) {
			cs_.cullingEnabled = true; }
		else if (value == GL_SCISSOR_TEST) {
			cs_.scissorEnabled = true; }
		else if (value == GL_BLEND) {
			cs_.blendingEnabled = true; }
		else if (value == GL_DEPTH_TEST) {
			cs_.depthTestEnabled = true; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void Disable(const int value) {
		dirty_ = true;
		if (value == GL_CULL_FACE) {
			cs_.cullingEnabled = false; }
		else if (value == GL_SCISSOR_TEST) {
			cs_.scissorEnabled = false; }
		else if (value == GL_BLEND) {
			cs_.blendingEnabled = false; }
		else if (value == GL_DEPTH_TEST) {
			cs_.depthTestEnabled = false; }
		else {
			throw std::runtime_error("unknown glEnable value"); }}

	void DepthFunc(int value) {
		dirty_ = true;
		cs_.depthFunc = value; }

	void DepthWriteMask(bool value) {
		dirty_ = true;
		cs_.depthWriteMask = value; }

	void ColorWriteMask(bool value) {
		dirty_ = true;
		cs_.colorWriteMask = value; }

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

	void RenderbufferType(int attachment, int type) {
		dirty_ = true;
		if (attachment == GL_DEPTH_ATTACHMENT) {
			cs_.depthAttachmentType = type; } 
		else if (attachment == GL_COLOR_ATTACHMENT0) {
			cs_.color0AttachmentType = type; }}

	auto AllocUniformBuffer() -> std::pair<int, void*> {
		constexpr int amt = UNIFORM_BUFFER_SIZE;
		int idx = static_cast<int>(ubuf_.size());
		for (int i=0; i<amt; i++) { ubuf_.emplace_back(); }
		auto* ptr = reinterpret_cast<void*>(ubuf_.data() + idx);
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

	void BindTexture3(const float* ptr, int dim) {
		dirty_ = true;
		cs_.tu3ptr = ptr;
		cs_.tu3dim = dim; }

	void UseBuffer(int idx, const Float3Array& ptr) {
		dirty_ = true;
		cs_.buffers[idx]   = ptr.x.data();
		cs_.buffers[idx+1] = ptr.y.data();
		cs_.buffers[idx+2] = ptr.z.data(); }

	void UseBuffer(int idx, const float* ptr) {
		dirty_ = true;
		cs_.buffers[idx] = ptr; }

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

	void ResetX() {
		dirty_ = true;
		cs_.reset(); }

	//inline void glUniform(const VertexInputUniform& viu) {
	//	state.vertex_input_uniform = viu; }

	//void drawElements(const VertexArray_F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3F3&, const rcls::vector<int>&);
	//void drawElements(const VertexArray_F3F3&);
	void DrawElements(int mode, int count, int type, const uint16_t* indices, uint8_t hint=0);
	void DrawArrays(int mode, int start, int count);
	void DrawElementsInstanced(int mode, int count, int type, const uint16_t* indices, int instanceCnt);
	void DrawArraysInstanced(int mode, int start, int count, int instanceCnt);
	void Clear(uint8_t bits);
	void StoreColor(rglr::FloatingPointCanvas *dst, bool downsample);
	void StoreColor(rglr::QFloat4Canvas *dst);
	void StoreColor(rglr::TrueColorCanvas *dst, bool enableGammaCorrection);
	void StoreDepth(float* dst);
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
	std::vector<float> ubuf_;
	std::deque<GLState> states_; };


}  // namespace rglv
}  // namespace rqdq
