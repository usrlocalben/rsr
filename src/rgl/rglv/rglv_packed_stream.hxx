#pragma once
#include <cassert>
#include <iostream>
#include <vector>

#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

constexpr int maxSizeInBytes = 1024 * 1024;


class FastPackedStream {
	int d_head{0};
	int d_tail{0};
	int d_mark1{0};
	int d_mark2{0};
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

	inline uint8_t *peek() const {
		return &d_buf[d_tail]; }

public:
	FastPackedStream()  {
		store.reserve(maxSizeInBytes);
		d_buf = store.data(); }

	inline bool eof()  const { return d_tail == d_head; }
	inline int  size() const { return d_head; }
	inline void reset()      { d_head = d_tail = 0; }

	inline void mark1()       { d_mark1 = d_head; }
	inline bool touched1()    { return d_mark1 != d_head; }
	inline void mark2()       { d_mark2 = d_head; }
	inline bool touched2()    { return d_mark2 != d_head; }

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

	inline auto peekUShort() const {
		return *reinterpret_cast<uint16_t*>(peek()); }

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


class FastPackedReader {
	// uint8_t* const base_;
	uint8_t* tail_;
	// const uint8_t* const end_;
public:
	FastPackedReader(uint8_t* begin) : //, uint8_t* end) :
		tail_(begin) {}
		// end_(end) {}

private:
	template<int MANY>
	inline uint8_t *Consume() {
		auto ptr = tail_;
		tail_ += MANY;
		return ptr; }

	inline uint8_t *Peek() const {
		return tail_; }

public:
	// inline bool Eof()  const { return tail_ == end_; }
	// inline int  Size() const { return end_ - base_; }

	inline auto ConsumeByte() {
		return *reinterpret_cast<uint8_t*>(Consume<sizeof(uint8_t)>()); }
	inline auto ConsumeUShort() {
		return *reinterpret_cast<uint16_t*>(Consume<sizeof(uint16_t)>()); }
	inline auto ConsumeInt() {
		return *reinterpret_cast<int*>(Consume<sizeof(int)>()); }
	inline auto ConsumeFloat() {
		return *reinterpret_cast<float*>(Consume<sizeof(float)>()); }
	inline auto ConsumePtr() {
		return *reinterpret_cast<void**>(Consume<sizeof(void*)>()); }
	inline auto ConsumeVec4() {
		auto x = ConsumeFloat();
		auto y = ConsumeFloat();
		auto z = ConsumeFloat();
		auto w = ConsumeFloat();
		return rmlv::vec4{ x, y, z, w }; }

	inline auto PeekUShort() const {
		return *reinterpret_cast<uint16_t*>(Peek()); } };


}  // namespace rglv
}  // namespace rqdq
