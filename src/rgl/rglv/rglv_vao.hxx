#pragma once
#include <rcls_aligned_containers.hxx>
#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>

#include <cassert>
#include <optional>

namespace rqdq {
namespace rglv {


struct Float2Array {
	rmlv::vec2 at(int idx) const {
		return rmlv::vec2{x[idx], y[idx]};}

	rmlv::qfloat2 load(const int idx) const {
		return rmlv::qfloat2{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]) };}

	int size() const {
		return int(x.size()); }

	inline void push_back(rmlv::vec2 a) {
		x.push_back(a.x); y.push_back(a.y);}

	void clear() {
		x.clear(); y.clear();}

	rcls::vector<float> x, y;
	};


struct Float3Array {
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

	inline void push_back(rmlv::vec3 a) {
		assert(x.size() == y.size() && y.size() == z.size());
		x.push_back(a.x); y.push_back(a.y); z.push_back(a.z); }

	void clear() {
		x.reserve(1024);
		y.reserve(1024);
		z.reserve(1024);
		x.clear(); y.clear(); z.clear();}

	rcls::vector<float> x, y, z; };


struct VertexArray_F3F3 {
	void clear() {
		a0.clear();
		a1.clear(); }

	int size() const {
		return int(a0.size());}

	int append(const rmlv::vec3& v0, const rmlv::vec3& v1) {
		int idx = size();
		a0.push_back(v0);
		a1.push_back(v1);
		return idx; }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			a0.push_back({0.0f});
			a1.push_back({0.0f});}}

	Float3Array a0;
	Float3Array a1; };


struct VertexArray_F3F3F2 {
	void clear() {
		a0.clear();
		a1.clear();
		a2.clear();}

	int size() const {
		return int(a0.size());}

	int append(rmlv::vec3 v0, rmlv::vec3 v1, rmlv::vec2 v2) {
		int idx = size();
		a0.push_back(v0);
		a1.push_back(v1);
		a2.push_back(v2);
		return idx; }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			a0.push_back({0.0f});
			a2.push_back({0.0f});
			a1.push_back({0.0f});}}

	Float3Array a0;
	Float3Array a1;
	Float2Array a2; };


struct VertexArray_F3F3F3 {
	void clear() {
		a0.clear();
		a1.clear();
		a2.clear();}

	int size() const {
		return int(a0.size());}

	int append(const rmlv::vec3 v0, const rmlv::vec3 v1, const rmlv::vec3 v2) {
		int idx = size();
		a0.push_back(v0);
		a1.push_back(v1);
		a2.push_back(v2);
		return idx; }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			a0.push_back({0.0f});
			a1.push_back({0.0f});
			a2.push_back({0.0f});}}

	Float3Array a0;
	Float3Array a1;
	Float3Array a2; };


}  // close package namespace
}  // close enterprise namespace
