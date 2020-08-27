#include "src/rgl/rglv/rglv_gl.hxx"

#include <cassert>
#include <cstdint>

#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

void GL::DrawArrays(const int mode [[maybe_unused]], const int start [[maybe_unused]], const int count) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ARRAYS);
	// commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count); }


void GL::DrawArraysInstanced(const int mode [[maybe_unused]], const int start [[maybe_unused]], const int count, int instanceCnt) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ARRAYS_INSTANCED);
	// commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendInt(instanceCnt); }


void GL::DrawElements(const int mode [[maybe_unused]], const int count, const int type [[maybe_unused]], const uint16_t* indices, const uint8_t hint) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ELEMENTS);
	commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	commands_.appendByte(hint);
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendPtr(indices); }


void GL::DrawElementsInstanced(const int mode [[maybe_unused]], const int count, const int type [[maybe_unused]], const uint16_t* indices, int instanceCnt) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ELEMENTS_INSTANCED);
	commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendPtr(indices);
	commands_.appendInt(instanceCnt); }


void GL::Clear(const uint8_t bits) {
	MaybeUpdateState();
	commands_.appendByte(CMD_CLEAR);
	commands_.appendByte(bits); }


void GL::StoreColor(rglr::FloatingPointCanvas *dst, bool downsample) {
	MaybeUpdateState();
	if (downsample) {
		commands_.appendByte(CMD_STORE_COLOR_HALF_LINEAR_FP); }
	else {
		commands_.appendByte(CMD_STORE_COLOR_FULL_LINEAR_FP); }
	commands_.appendPtr(dst); }


void GL::StoreColor(rglr::QFloat4Canvas* dst) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_COLOR_FULL_QUADS_FP);
	commands_.appendPtr(dst); }


void GL::StoreColor(rglr::TrueColorCanvas* dst, bool enableGammaCorrection) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_COLOR_FULL_LINEAR_TC);
	commands_.appendByte(static_cast<uint8_t>(enableGammaCorrection));
	commands_.appendPtr(dst); }


void GL::StoreDepth(float* dst) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_DEPTH_FULL_LINEAR_FP);
	commands_.appendPtr(dst); }


void GL::Reset() {
	dirty_ = true;
	states_.clear();
	cs_.reset();
	ubuf_.clear();
	commands_.reset(); }


void GL::MaybeUpdateState() {
	if (!dirty_) {
		return; }
	states_.push_back(cs_);
	commands_.appendByte(CMD_STATE);
	commands_.appendPtr(&states_.back());
	dirty_ = false; }


}  // namespace rglv
}  // namespace rqdq
