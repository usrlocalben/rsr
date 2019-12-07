#pragma once
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_mvec4.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"

#include <array>

namespace rqdq {
namespace rmlm {

struct qmat4 {
	std::array<rmlv::mvec4f, 16> f;  // column-major!

	qmat4() = default;
	explicit qmat4(const mat4& m); };


auto mul_w1(const qmat4& a, const rmlv::qfloat3& b) -> rmlv::qfloat4;
auto mul_w0(const qmat4& a, const rmlv::qfloat3& b) -> rmlv::qfloat3;
auto mul_w0(const qmat4& a, const rmlv::qfloat4& b) -> rmlv::qfloat3;
auto operator*(const qmat4& a, const rmlv::qfloat4& b) -> rmlv::qfloat4;


inline
qmat4::qmat4(const mat4& m) :f({ {
		/*these are transposed, the indices match openGL*/
		rmlv::mvec4f{m.ff[0x00]}, rmlv::mvec4f{m.ff[0x01]}, rmlv::mvec4f{m.ff[0x02]}, rmlv::mvec4f{m.ff[0x03]},
		rmlv::mvec4f{m.ff[0x04]}, rmlv::mvec4f{m.ff[0x05]}, rmlv::mvec4f{m.ff[0x06]}, rmlv::mvec4f{m.ff[0x07]},
		rmlv::mvec4f{m.ff[0x08]}, rmlv::mvec4f{m.ff[0x09]}, rmlv::mvec4f{m.ff[0x0a]}, rmlv::mvec4f{m.ff[0x0b]},
		rmlv::mvec4f{m.ff[0x0c]}, rmlv::mvec4f{m.ff[0x0d]}, rmlv::mvec4f{m.ff[0x0e]}, rmlv::mvec4f{m.ff[0x0f]} } }) {}


inline
auto mul_w1(const qmat4& a, const rmlv::qfloat3& b) -> rmlv::qfloat4 {
	return {
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z + a.f[12],
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z + a.f[13],
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z + a.f[14],
		a.f[3]*b.x + a.f[7]*b.y + a.f[11]*b.z + a.f[15] }; }


inline
auto mul_w0(const qmat4& a, const rmlv::qfloat3& b) -> rmlv::qfloat3 {
	return {
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z,
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z,
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z }; }


inline
auto mul_w0(const qmat4& a, const rmlv::qfloat4& b) -> rmlv::qfloat3 {
	return {
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z,
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z,
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z }; }


inline
auto operator*(const qmat4& a, const rmlv::qfloat4& b) -> rmlv::qfloat4 {
	return {
		a.f[0]*b.x + a.f[4]*b.y + a.f[ 8]*b.z + a.f[12]*b.w,
		a.f[1]*b.x + a.f[5]*b.y + a.f[ 9]*b.z + a.f[13]*b.w,
		a.f[2]*b.x + a.f[6]*b.y + a.f[10]*b.z + a.f[14]*b.w,
		a.f[3]*b.x + a.f[7]*b.y + a.f[11]*b.z + a.f[15]*b.w }; }


}  // namespace rmlm
}  // namespace rqdq
