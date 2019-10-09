#pragma once
#include <algorithm>
#include <cassert>
#include <optional>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

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

	rmlv::qfloat2 loadxy(const int idx) const {
		return rmlv::qfloat2{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]) }; }

	rmlv::qfloat4 loadxyz1(const int idx) const {
		return rmlv::qfloat4{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]), _mm_set1_ps(1.0F) };}

	rmlv::qfloat4 loadxyz0(const int idx) const {
		return rmlv::qfloat4{ _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]), _mm_set1_ps(0.0F) };}

	int size() const {
		assert(x.size() == y.size() && y.size() == z.size());
		return int(x.size()); }

	void add(int idx, rmlv::vec3 d) {
		x[idx] += d.x;
		y[idx] += d.y;
		z[idx] += d.z; }

	void set(int idx, rmlv::vec3 d) {
		x[idx] = d.x;
		y[idx] = d.y;
		z[idx] = d.z; }

	inline void push_back(rmlv::vec3 a) {
		assert(x.size() == y.size() && y.size() == z.size());
		x.push_back(a.x); y.push_back(a.y); z.push_back(a.z); }

	void reserve(int num) {
		x.reserve(num);
		y.reserve(num);
		z.reserve(num); }

	void resize(int num) {
		x.resize(num, 0.0F);
		y.resize(num, 0.0F);
		z.resize(num, 0.0F); }

	void clear() {
		x.clear();
		y.clear();
		z.clear();}

	void zero() {
		std::fill(begin(x), end(x), 0.0F);
		std::fill(begin(y), end(y), 0.0F);
		std::fill(begin(z), end(z), 0.0F); }

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
			a0.push_back({0.0F});
			a1.push_back({0.0F});}}

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
			a0.push_back({0.0F});
			a2.push_back({0.0F});
			a1.push_back({0.0F});}}

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

	void resize(int num) {
		a0.resize(num);
		a1.resize(num);
		a2.resize(num); }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			a0.push_back({0.0F});
			a1.push_back({0.0F});
			a2.push_back({0.0F});}}

	Float3Array a0;
	Float3Array a1;
	Float3Array a2; };


}  // namespace rglv
}  // namespace rqdq
