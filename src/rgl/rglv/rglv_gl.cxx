#include "src/rgl/rglv/rglv_gl.hxx"

#include <cassert>

#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

void GL::glDrawArrays(const int mode, const int start, const int count) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	maybeUpdateState();
	d_commands.appendByte(CMD_DRAW_ARRAY);
	// d_commands.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// d_commands.appendByte(enableClipping ? 1 : 0);
	d_commands.appendInt(count); }


void GL::glDrawElements(const int mode, const int count, const int type, const uint16_t* indices) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	maybeUpdateState();
	d_commands.appendByte(CMD_DRAW_ELEMENTS);
	d_commands.appendByte(0x14);  // videocore: 16-bit indices, triangles
	// d_commands.appendByte(enableClipping ? 1 : 0);
	d_commands.appendInt(count);
	d_commands.appendPtr(indices); }


void GL::glClear(const int bits) {
	maybeUpdateState();
	d_commands.appendByte(CMD_CLEAR);
	d_commands.appendByte(bits); }


void GL::storeHalfsize(rglr::FloatingPointCanvas *dst) {
	maybeUpdateState();
	d_commands.appendByte(CMD_STORE_FP32_HALF);
	d_commands.appendPtr(dst); }


void GL::storeUnswizzled(rglr::FloatingPointCanvas *dst) {
	maybeUpdateState();
	d_commands.appendByte(CMD_STORE_FP32);
	d_commands.appendPtr(dst); }


void GL::storeTrueColor(bool enableGammaCorrection, rglr::TrueColorCanvas* dst) {
	maybeUpdateState();
	d_commands.appendByte(CMD_STORE_TRUECOLOR);
	d_commands.appendByte(static_cast<uint8_t>(enableGammaCorrection));
	d_commands.appendPtr(dst); }


void GL::reset() {
	d_dirty = true;
	d_states.clear();
	d_cs.reset();
	d_ubuf.clear();
	d_commands.reset(); }


void GL::maybeUpdateState() {
	if (!d_dirty) {
		return; }
	d_states.push_back(d_cs);
	d_commands.appendByte(CMD_STATE);
	d_commands.appendPtr(&d_states.back());
	d_dirty = false; }


}  // namespace rglv
}  // namespace rqdq
