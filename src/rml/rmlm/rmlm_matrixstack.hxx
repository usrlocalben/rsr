#pragma once
#include <array>
#include <cassert>

namespace rqdq {
namespace rmlm {

template <typename T, int DEPTH>
class MatrixStack {
	static constexpr int maxDepth = DEPTH;
	using value_type = T;

    // DATA
	std::array<T, maxDepth> buf_;
	int sp_{0};

public:
    MatrixStack();
    MatrixStack(const T& init);

    // MANIPULATORS
	void Push();
	void Pop();
	auto Top() const -> const T&;
	void Clear();
	void Mul(const T& m);
	void Load(const T& m);
	void Reset(); };


// ============================================================================
//                             INLINE DEFINITIONS
// ============================================================================

							// -----------------
							// class MatrixStack
							// -----------------

// CREATORS
template <typename T, int DEPTH>
inline
MatrixStack<T, DEPTH>::MatrixStack() = default;

template <typename T, int DEPTH>
inline
MatrixStack<T, DEPTH>::MatrixStack(const T& init) {
    buf_[0] = init; }

// MANIPULATORS
template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Push() {
	assert(sp_+1 < maxDepth);
	buf_[sp_+1] = buf_[sp_];
	++sp_; }

template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Pop() {
	assert(sp_ > 0);
	--sp_; }

template <typename T, int DEPTH>
inline
auto MatrixStack<T, DEPTH>::Top() const -> const T& {
	return buf_[sp_]; }

template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Clear() {
	sp_ = 0; }

template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Mul(const T& m) {
	buf_[sp_] *= m; }

template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Load(const T& m) {
	buf_[sp_] = m; }

template <typename T, int DEPTH>
inline
void MatrixStack<T, DEPTH>::Reset() {
	sp_ = 0;
	Load(T{1}); }


}  // namespace rmlm
}  // namespace rqdq
