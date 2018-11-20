#pragma once
#include <rcls_aligned_containers.hxx>
#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>

#include <cassert>
#include <optional>

namespace rqdq {
namespace rglv {


struct SoArray_Float2 {
	rmlv::vec2 at(int idx) const {
		return rmlv::vec2{x[idx], y[idx]};}

	rmlv::qfloat2 load(const int idx) const {
		return rmlv::qfloat2{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]) };}

	int size() const {
		return int(x.size()); }

	void push_back(rmlv::vec2 a) {
		x.push_back(a.x); y.push_back(a.y);}

	void clear() {
		x.clear(); y.clear();}

	rcls::vector<float> x, y;
	};


struct SoArray_Float3 {
	rmlv::vec3 at(int idx) const {
		return rmlv::vec3{x[idx], y[idx], z[idx]}; }

	rmlv::qfloat3 load(const int idx) const {
		return rmlv::qfloat3{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]) };}

	rmlv::qfloat4 loadxyz1(const int idx) const {
		return rmlv::qfloat4{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]), _mm_set1_ps(1.0f) };}

	rmlv::qfloat4 loadxyz0(const int idx) const {
		return rmlv::qfloat4{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]), _mm_set1_ps(0.0f) };}

	int size() const {
		assert(x.size() == y.size() && y.size() == z.size());
		return int(x.size()); }

	void push_back(rmlv::vec3 a) {
		assert(x.size() == y.size() && y.size() == z.size());
		x.push_back(a.x); y.push_back(a.y); z.push_back(a.z); }

	void clear() {
		x.reserve(1024);
		y.reserve(1024);
		z.reserve(1024);
		x.clear(); y.clear(); z.clear();}

	rcls::vector<float> x, y, z; };


struct VertexArray_PN {
	std::optional<int> find(const rmlv::vec3& p, const rmlv::vec3& n) const {
		for (int i = 0; i < size(); i++) {
			if (almost_equal(position.at(i), p) &&
				almost_equal(normal.at(i), n))
				return i; }
		return {}; }

	int append(const rmlv::vec3& p, const rmlv::vec3& n) {
		position.push_back(p);
		normal.push_back(n);
		return size() - 1; }

	void clear() {
		position.clear();
		normal.clear(); }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			position.push_back({0.0f});
			normal.push_back({0.0f});}}

	int upsert(const rmlv::vec3& p, const rmlv::vec3& n) {
		if (auto existing_idx = find(p, n)) {
			return existing_idx.value(); }
		return append(p, n); }

	int size() const {
		return int(position.size());}

	SoArray_Float3 position;
	SoArray_Float3 normal; };


struct VertexArray_PNT {
	std::optional<int> find(const rmlv::vec3& p, const rmlv::vec3& n, const rmlv::vec2& t) const {
		for (int i = 0; i < size(); i++) {
			if (almost_equal(position.at(i), p) &&
				almost_equal(normal.at(i), n) &&
				almost_equal(texcoord.at(i), t))
				return i; }
		return {}; }

	int append(const rmlv::vec3& p, const rmlv::vec3& n, const rmlv::vec2& t) {
		position.push_back(p);
		normal.push_back(n);
		texcoord.push_back(t);
		return size() - 1; }

	void clear() {
		position.clear();
		normal.clear();
		texcoord.clear();}

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			position.push_back({0.0f});
			texcoord.push_back({0.0f});
			normal.push_back({0.0f});}}

	int upsert(const rmlv::vec3& p, const rmlv::vec3& n, const rmlv::vec2& t) {
		if (auto existing_idx = find(p, n, t)) {
			return existing_idx.value(); }
		return append(p, n, t); }

	int size() const {
		return int(position.size());}

	SoArray_Float3 position;
	SoArray_Float3 normal;
	SoArray_Float2 texcoord;
	};


struct VertexArray_PNM {
	std::optional<int> find(const rmlv::vec3 p, const rmlv::vec3 n, const rmlv::vec3 a) const {
		for (int i = 0; i < size(); i++) {
			if (almost_equal(position.at(i), p) &&
				almost_equal(normal.at(i), n) &&
				almost_equal(float3_1.at(i), a))
				return i; }
		return {}; }

	int append(const rmlv::vec3 p, const rmlv::vec3 n, const rmlv::vec3 a) {
		position.push_back(p);
		normal.push_back(n);
		float3_1.push_back(a);
		return size() - 1; }

	void clear() {
		position.clear();
		normal.clear();
		float3_1.clear();}

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			position.push_back({0.0f});
			float3_1.push_back({0.0f});
			normal.push_back({0.0f});}}

	int upsert(const rmlv::vec3 p, const rmlv::vec3 n, const rmlv::vec3 a) {
		if (auto existing_idx = find(p, n, a)) {
			return existing_idx.value(); }
		return append(p, n, a); }

	int size() const {
		return int(position.size());}

	SoArray_Float3 position;
	SoArray_Float3 normal;
	SoArray_Float3 float3_1; };


}  // close package namespace
}  // close enterprise namespace
