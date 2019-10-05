#pragma once
#include <cstdint>

namespace rqdq {
namespace rglv {

constexpr uint8_t CMD_EOF = 0x8d;

// void* follows with GPUState addr
constexpr uint8_t CMD_STATE = 1;


// draw commands
// vec4 follows with color
constexpr uint8_t CMD_CLEAR = 10;

// vao ptr, idx ptr
constexpr uint8_t CMD_DRAW_ELEMENTS = 11;

constexpr uint8_t CMD_DRAW_ARRAYS = 12;

constexpr uint8_t CMD_DRAW_ELEMENTS_INSTANCED = 13;

constexpr uint8_t CMD_DRAW_ARRAYS_INSTANCED = 14;

// 0xffff terminated list of unsigned int indexed GL_TRIANGLES
constexpr uint8_t CMD_DRAW_INLINE = 15;

// like CMD_DRAW_INLINE, but each triangle is prefixed with
// a uint16_t instanceId
constexpr uint8_t CMD_DRAW_INLINE_INSTANCED = 16;

// buffer commands
// ptr to floatingpointcanvas
constexpr uint8_t CMD_STORE_FP32 = 20;

// ptr to floatingpointcanvas
constexpr uint8_t CMD_STORE_FP32_HALF = 21;

// byte bool enable/disable srgb, ptr to truecolorcanvas
constexpr uint8_t CMD_STORE_TRUECOLOR = 22;


// internal use below this point
constexpr uint8_t CMD_CLIPPED_TRI = 100;

constexpr int AF_FLOAT = 1;  // float*
constexpr int AF_VAO_F3F3F3 = 2;  // VertexArray_F3F3F3


}  // namespace rglv
}  // namespace rqdq
