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

constexpr uint8_t CMD_DRAW_ARRAY = 12;

// 0xffff terminated list of unsigned int indexed GL_TRIANGLES
constexpr uint8_t CMD_DRAW_INLINE = 13;

// buffer commands
// ptr to floatingpointcanvas
constexpr uint8_t CMD_STORE_FP32 = 20;

// ptr to floatingpointcanvas
constexpr uint8_t CMD_STORE_FP32_HALF = 21;

// byte bool enable/disable srgb, ptr to truecolorcanvas
constexpr uint8_t CMD_STORE_TRUECOLOR = 22;


// internal use below this point
constexpr uint8_t CMD_CLIPPED_TRI = 100;

constexpr int AF_VAO_F3F3F3 = 1;  // VertexArray_F3F3F3

}  // close package namespace
}  // close enterprise namespace
