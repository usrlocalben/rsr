#pragma once
#include <cassert>
#include <iostream>
#include <vector>

#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

constexpr int maxSizeInBytes = 100000;


class FastPackedStream {
	int d_head{0};
	int d_tail{0};
	int d_mark{0};
	std::vector<uint8_t> store;
	uint8_t* d_buf;

	inline uint8_t *alloc(int many) {
		auto head = d_head;
		d_head += many;
		assert(d_head < maxSizeInBytes);
		return &d_buf[head]; }

	inline uint8_t *consume(int many) {
		auto ptr = &d_buf[d_tail];
		d_tail += many;
		return ptr; }

public:
	FastPackedStream()  {
		store.reserve(maxSizeInBytes);
		d_buf = store.data(); }

	inline bool eof()  const { return d_tail == d_head; }
	inline int  size() const { return d_head; }
	inline void reset()      { d_head = d_tail = 0; }

	inline void mark()       { d_mark = d_head; }
	inline bool touched()    { return d_mark != d_head; }

	inline void appendByte(uint8_t a) {
		*reinterpret_cast<uint8_t*>(alloc(sizeof(uint8_t))) = a; }
	inline void appendUShort(uint16_t a) {
		*reinterpret_cast<uint16_t*>(alloc(sizeof(uint16_t))) = a; }
	inline void appendInt(int a) {
		*reinterpret_cast<int*>(alloc(sizeof(int))) = a; }
	inline void appendFloat(float a) {
		*reinterpret_cast<float*>(alloc(sizeof(float))) = a; }
	inline void appendPtr(const void * const a) {
		*reinterpret_cast<const void**>(alloc(sizeof(void*))) = a; }
	inline void appendVec4(rmlv::vec4 a) {
		appendFloat(a.x);
		appendFloat(a.y);
		appendFloat(a.z);
		appendFloat(a.w); }

	inline auto consumeByte() {
		return *reinterpret_cast<uint8_t*>(consume(sizeof(uint8_t))); }
	inline auto consumeUShort() {
		return *reinterpret_cast<uint16_t*>(consume(sizeof(uint16_t))); }
	inline auto consumeInt() {
		return *reinterpret_cast<int*>(consume(sizeof(int))); }
	inline auto consumeFloat() {
		return *reinterpret_cast<float*>(consume(sizeof(float))); }
	inline auto consumePtr() {
		return *reinterpret_cast<void**>(consume(sizeof(void*))); }
	inline auto consumeVec4() {
		auto x = consumeFloat();
		auto y = consumeFloat();
		auto z = consumeFloat();
		auto w = consumeFloat();
		return rmlv::vec4{ x, y, z, w }; }

	inline auto unappend(int many) {
		d_head -= many; } };


}  // namespace rglv
}  // namespace rqdq
