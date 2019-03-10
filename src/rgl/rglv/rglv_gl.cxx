#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rgl/rglv/rglv_gpu_protocol.hxx"
#include "src/rgl/rglv/rglv_packed_stream.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <cassert>

namespace rqdq {
namespace rglv {

void GL::glDrawArrays(const int mode, const int start, const int count, const bool enableClipping) {
	assert(mode == GL_TRIANGLES);
	assert(start == 0);
	maybeUpdateState();
	d_commands.appendByte(CMD_DRAW_ARRAY);
	// d_commands.appendByte(0x14);  // videocore: 16-bit indices, triangles
	d_commands.appendByte(enableClipping ? 1 : 0);
	d_commands.appendInt(count); }


void GL::glDrawElements(const int mode, const int count, const int type, const uint16_t* indices, const bool enableClipping) {
	assert(mode == GL_TRIANGLES);
	assert(type == GL_UNSIGNED_SHORT);
	maybeUpdateState();
	d_commands.appendByte(CMD_DRAW_ELEMENTS);
	d_commands.appendByte(0x14);  // videocore: 16-bit indices, triangles
	d_commands.appendByte(enableClipping ? 1 : 0);
	d_commands.appendInt(count);
	d_commands.appendPtr(indices); }


void GL::glClear(const rmlv::vec4 color) {
	maybeUpdateState();
	d_commands.appendByte(CMD_CLEAR);
	d_commands.appendVec4(color); }


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
	d_commands.appendByte(enableGammaCorrection);
	d_commands.appendPtr(dst); }


void GL::reset() {
	d_dirty = true;
	d_modelViewMatrixStack.reset();
	d_projectionMatrixStack.reset();
	d_textureMatrixStack.reset();
	d_activeMatrixStack = &d_modelViewMatrixStack;
	d_states.clear();
	d_cs.reset();
	d_commands.reset(); }


void GL::maybeUpdateState() {
	if (!d_dirty) return;
	d_cs.modelViewMatrix = d_modelViewMatrixStack.top();
	d_cs.projectionMatrix = d_projectionMatrixStack.top();
	d_cs.textureMatrix = d_textureMatrixStack.top();
	d_states.push_back(d_cs);
	d_commands.appendByte(CMD_STATE);
	d_commands.appendPtr(&d_states.back());
	d_dirty = false; }

}  // close package namespace
}  // close enterprise namespace
