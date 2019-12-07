#pragma once
#include <pmmintrin.h>

namespace rqdq {
namespace rmlv {

/**
 * _mm_shufd replacement for SSE2 from LiraNuna
 * https://github.com/LiraNuna/glsl-sse2/blob/master/source/mat4.h
 */
#define _mm_shufd(xmm,mask) _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(xmm),mask))


/**
 * signed 32-bit integer mul with low 32 bit result
 *
 * sse2 compatible.  sse4.1 can use _mm_mullo_epi32 instead
 *
 * http://stackoverflow.com/questions/10500766/sse-multiplication-of-4-32-bit-integers
 *
 */
inline
auto sse2_mul32(__m128i a, __m128i b) -> __m128i {
	// mul 2, 0
	const __m128i tmp1 = _mm_mul_epu32(a, b);

	// mul 3, 1
	const __m128i tmp2 = _mm_mul_epu32(_mm_srli_si128(a, 4), _mm_srli_si128(b, 4));

	// shuffle results to [63...0] and pack
	return _mm_unpacklo_epi32(
		_mm_shuffle_epi32(tmp1, _MM_SHUFFLE(0, 0, 2, 0)),
		_mm_shuffle_epi32(tmp2, _MM_SHUFFLE(0, 0, 2, 0))
		); }


}  // namespace rmlv
}  // namespace rqdq
