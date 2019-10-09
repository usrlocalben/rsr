/**
 * http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
 */
#pragma once
#include <cmath>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglv {

class Icosphere {
public:
	Icosphere(int divs);

	int GetNumVertices() const {
		return vCnt_; }

	int GetNumIndices() const {
		return tris_.size(); }

	const uint16_t* GetIndices() const {
		const uint16_t* foo = tris_.data();
		return foo; }

	const float* GetBuffer(int n) const {
		if (n==0) return px.data();
		if (n==1) return py.data();
		if (n==2) return pz.data();
		throw std::runtime_error("bad buffer number"); }

	// get point
	rmlv::vec3 GP(int i) const {
		return rmlv::vec3{ px[i], py[i], pz[i] }; }

private:
	void Sphereize();
	void Pad();
	void Optimize();

	int AP(float x, float y, float z);
	void UP(int i, rmlv::vec3 d);
	int MP(int i0, int i1, rmlv::vec3 p0, rmlv::vec3 p1);
	void F(int i0, int i1, int i2);

private:
	int vCnt_;
	rcls::vector<float> px, py, pz;
	std::unordered_map<uint64_t, int> edges_;
	std::vector<uint16_t> tris_;
	std::optional<std::vector<uint16_t>> tmps_; };


}  // namespace rglv
}  // namespace rqdq

