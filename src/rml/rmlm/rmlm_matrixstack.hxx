#pragma once
#include <array>
#include <cassert>

namespace rqdq {
namespace rmlm {

template<class T, int DEPTH>
class MatrixStack {
	static constexpr int maxDepth = DEPTH;
	using elementType = T;

	std::array<elementType, maxDepth> buf_;
	int sp_{0};

public:
	void push();
	void pop();
	auto top() const -> const elementType& {
	void clear();
	void mul(const T& m);
	void load(const elementType& m);
	void reset(); };


template<class T, int DEPTH>
inline
void MatrixStack::push() {
	assert(sp_+1 < maxDepth);
	buf_[sp_+1] = buf_[sp_];
	++sp_; }


template<class T, int DEPTH>
inline
void MatrixStack::pop() {
	assert(sp_ > 0);
	--sp_; }


template<class T, int DEPTH>
inline
auto MatrixStack::top() const -> const MatrixStack::elementType& {
	return buf_[sp_]; }


template<class T, int DEPTH>
inline
void MatrixStack::clear() {
	sp_ = 0; }


template<class T, int DEPTH>
inline
void MatrixStack::mul(const MatrixStack::elementType& m) {
	buf_[sp_] *= m; }


template<class T, int DEPTH>
inline
void MatrixStack::load(const MatrixStack::elementType& m) {
	buf_[sp_] = m; }


template<class T, int DEPTH>
inline
void MatrixStack::reset() {
	sp_ = 0;
	load(mat4::identity()); }


}  // namespace rmlm
}  // namespace rqdq
