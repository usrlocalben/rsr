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

	template<int MANY>
	inline uint8_t *alloc() {
		auto pos = d_head;
		d_head += MANY;
		assert(d_head < maxSizeInBytes);
		return &d_buf[pos]; }

	template<int MANY>
	inline uint8_t *consume() {
		auto ptr = &d_buf[d_tail];
		d_tail += MANY;
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
		*reinterpret_cast<uint8_t*>(alloc<sizeof(uint8_t)>()) = a; }
	inline void appendUShort(uint16_t a) {
		*reinterpret_cast<uint16_t*>(alloc<sizeof(uint16_t)>()) = a; }
	inline void appendInt(int a) {
		*reinterpret_cast<int*>(alloc<sizeof(int)>()) = a; }
	inline void appendFloat(float a) {
		*reinterpret_cast<float*>(alloc<sizeof(float)>()) = a; }
	inline void appendPtr(const void * const a) {
		*reinterpret_cast<const void**>(alloc<sizeof(void*)>()) = a; }
	inline void appendVec4(rmlv::vec4 a) {
		appendFloat(a.x);
		appendFloat(a.y);
		appendFloat(a.z);
		appendFloat(a.w); }

	inline auto consumeByte() {
		return *reinterpret_cast<uint8_t*>(consume<sizeof(uint8_t)>()); }
	inline auto consumeUShort() {
		return *reinterpret_cast<uint16_t*>(consume<sizeof(uint16_t)>()); }
	inline auto consumeInt() {
		return *reinterpret_cast<int*>(consume<sizeof(int)>()); }
	inline auto consumeFloat() {
		return *reinterpret_cast<float*>(consume<sizeof(float)>()); }
	inline auto consumePtr() {
		return *reinterpret_cast<void**>(consume<sizeof(void*)>()); }
	inline auto consumeVec4() {
		auto x = consumeFloat();
		auto y = consumeFloat();
		auto z = consumeFloat();
		auto w = consumeFloat();
		return rmlv::vec4{ x, y, z, w }; }

	inline auto unappend(int many) {
		d_head -= many; } };


class FastPackedWriter {
	uint8_t* const base_;
	uint8_t* head_;
	// const int limit_;
	uint8_t* mark_{nullptr};

public:
	FastPackedWriter(uint8_t* base) :
		base_(base),
		head_(base_) {}

private:
	template<int MANY>
	uint8_t* alloc() {
		auto pos = head_;
		head_ += MANY;
		// assert(Size() < limit_);
		return pos; }

	inline int  Size() const { return head_ - base_; }

	inline void Mark()       { mark_ = head_; }
	inline bool Touched()    { return mark_ != head_; }

	inline void AppendByte(uint8_t a) {
		*reinterpret_cast<uint8_t*>(alloc<sizeof(uint8_t)>()) = a; }
	inline void AppendUShort(uint16_t a) {
		*reinterpret_cast<uint16_t*>(alloc<sizeof(uint16_t)>()) = a; }
	inline void AppendInt(int a) {
		*reinterpret_cast<int*>(alloc<sizeof(int)>()) = a; }
	inline void AppendFloat(float a) {
		*reinterpret_cast<float*>(alloc<sizeof(float)>()) = a; }
	inline void AppendPtr(const void * const a) {
		*reinterpret_cast<const void**>(alloc<sizeof(void*)>()) = a; }
	inline void AppendVec4(rmlv::vec4 a) {
		AppendFloat(a.x);
		AppendFloat(a.y);
		AppendFloat(a.z);
		AppendFloat(a.w); }

	inline auto Unappend(int many) {
		head_ -= many; } };


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
		return rmlv::vec4{ x, y, z, w }; } };


}  // namespace rglv
}  // namespace rqdq
