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
	rcls::vector<float> x, y;

	auto at(int idx) const -> rmlv::vec2 {
		return {x[idx], y[idx]};}

	auto load(const int idx) const -> rmlv::qfloat2 {
		return { _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]) };}

	auto size() const -> int {
		return static_cast<int>(x.size()); }

	void push_back(rmlv::vec2 a) {
		x.push_back(a.x); y.push_back(a.y);}

	void reserve(int n) {
		x.reserve(n); y.reserve(n); }

	void resize(int n) {
		x.resize(n, 0.0F); y.resize(n, 0.0F); }

	void clear() {
		x.clear(); y.clear();}

	void zero() {
		std::fill(begin(x), end(x), 0.0F);
		std::fill(begin(y), end(y), 0.0F); }};


struct Float3Array {
	rcls::vector<float> x, y, z;

	auto at(int idx) const -> rmlv::vec3 {
		return {x[idx], y[idx], z[idx]}; }

	auto load(const int idx) const -> rmlv::qfloat3 {
		return { _mm_load_ps(&x[idx]), _mm_load_ps(&y[idx]), _mm_load_ps(&z[idx]) };}

	auto size() const -> int {
		assert(x.size() == y.size() && y.size() == z.size());
		return static_cast<int>(x.size()); }

	void add(int idx, rmlv::vec3 d) {
		x[idx] += d.x; y[idx] += d.y; z[idx] += d.z; }

	void set(int idx, rmlv::vec3 d) {
		x[idx] = d.x; y[idx] = d.y; z[idx] = d.z; }

	void push_back(rmlv::vec3 a) {
		assert(x.size() == y.size() && y.size() == z.size());
		x.push_back(a.x); y.push_back(a.y); z.push_back(a.z); }

	void reserve(int n) {
		x.reserve(n); y.reserve(n); z.reserve(n); }

	void resize(int n) {
		x.resize(n, 0.0F); y.resize(n, 0.0F); z.resize(n, 0.0F); }

	void clear() {
		x.clear(); y.clear(); z.clear();}

	void zero() {
		std::fill(begin(x), end(x), 0.0F);
		std::fill(begin(y), end(y), 0.0F);
		std::fill(begin(z), end(z), 0.0F); }};


struct VertexArray_F3F3 {
	Float3Array a0;
	Float3Array a1;

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

	void resize(int num) {
		a0.resize(num);
		a1.resize(num); }

	void pad() {
		const int rag = size() % 4;
		const int needed = 4 - rag;
		for (int i = 0; i < needed; i++) {
			a0.push_back({0.0F});
			a1.push_back({0.0F});}}};



struct VertexArray_F3F3F2 {
	Float3Array a0;
	Float3Array a1;
	Float2Array a2;

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
			a1.push_back({0.0F});}}};


struct VertexArray_F3F3F3 {
	Float3Array a0;
	Float3Array a1;
	Float3Array a2;

	void clear() {
		a0.clear();
		a1.clear();
		a2.clear();}

	int size() const {
		return int(a0.size());}

	bool empty() const {
		return size()==0; }

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
			a2.push_back({0.0F});}}};


}  // namespace rglv
}  // namespace rqdq
