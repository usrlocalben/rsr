#pragma once

namespace rqdq {
namespace rmlg {


inline bool is_pow2(unsigned x) {
	while (((x & 1) == 0) && x > 1) {
		x >>= 1; }
	return x == 1; }


inline int ilog2(unsigned x) {
	int pow = 0;
	while (x != 0U) {
		x >>= 1;
		pow++; }
	return pow - 1; }


inline int pow2ceil(int a) {
	int c = 1;
	while (c < a) {
		c <<= 1; }
	return c; }

/*
inline float mFast_Log2(float val) {
	union { float val; int32_t x; } u = { val };
	register float log_2 = (float)(((u.x >> 23) & 255) - 128);
	u.x &= ~(255 << 23);
	u.x += 127 << 23;
	log_2 += ((-0.3358287811f) * u.val + 2.0f) * u.val - 0.65871759316667f;
	return (log_2); }
*/

}  // namespace rmlg
}  // namespace rqdq
