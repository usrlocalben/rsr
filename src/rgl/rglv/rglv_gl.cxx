#include "src/rgl/rglv/rglv_gl.hxx"

#include <cassert>

#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

void GL::DrawArrays(const int mode, const int start, const int count) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ARRAYS);
	// commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count); }


void GL::DrawArraysInstanced(const int mode, const int start, const int count, int instanceCnt) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ARRAYS_INSTANCED);
	// commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendInt(instanceCnt); }


void GL::DrawElements(const int mode, const int count, const int type, const uint16_t* indices) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ELEMENTS);
	commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendPtr(indices); }


void GL::DrawElementsInstanced(const int mode, const int count, const int type, const uint16_t* indices, int instanceCnt) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	MaybeUpdateState();
	commands_.appendByte(CMD_DRAW_ELEMENTS_INSTANCED);
	commands_.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// commands_.appendByte(enableClipping ? 1 : 0);
	commands_.appendInt(count);
	commands_.appendPtr(indices);
	commands_.appendInt(instanceCnt); }


void GL::Clear(const int bits) {
	MaybeUpdateState();
	commands_.appendByte(CMD_CLEAR);
	commands_.appendByte(bits); }


void GL::StoreHalfsize(rglr::FloatingPointCanvas *dst) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_FP32_HALF);
	commands_.appendPtr(dst); }


void GL::StoreUnswizzled(rglr::FloatingPointCanvas *dst) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_FP32);
	commands_.appendPtr(dst); }


void GL::StoreTrueColor(bool enableGammaCorrection, rglr::TrueColorCanvas* dst) {
	MaybeUpdateState();
	commands_.appendByte(CMD_STORE_TRUECOLOR);
	commands_.appendByte(static_cast<uint8_t>(enableGammaCorrection));
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
