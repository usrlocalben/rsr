#include "src/rgl/rglv/rglv_icosphere.hxx"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rml/rmlv/rmlv_math.hxx"

namespace rqdq {
namespace rglv {

Icosphere::Icosphere(int divs, int reqColor) {
	using rmlv::vec3;
	std::vector<int> tmpt;
	auto F = [&](int i0, int i1, int i2, int color) {
		tmpt.emplace_back(i0);
		tmpt.emplace_back(i1);
		tmpt.emplace_back(i2);
		tmpt.emplace_back(color); };

	// 1. Create an icosohedron
	const float t = (1.0F + sqrt(5.0F)) / 2.0F;

	const auto p0 = AP( -1.0F,  t, 0.0F );
	const auto p1 = AP(  1.0F,  t, 0.0F );
	const auto p2 = AP( -1.0F, -t, 0.0F );
	const auto p3 = AP(  1.0F, -t, 0.0F );

	const auto p4 = AP( 0, -1,  t );
	const auto p5 = AP( 0,  1,  t );
	const auto p6 = AP( 0, -1, -t );
	const auto p7 = AP( 0,  1, -t );

	const auto p8 = AP(    t, 0, -1 );
	const auto p9 = AP(    t, 0,  1 );
	const auto pA = AP(   -t, 0, -1 );
	const auto pB = AP(   -t, 0,  1 );

	tmpt.reserve(5*4*4);

	F(p0, pB, p5, 0);
	F(p0, p5, p1, 0);
	F(p0, p1, p7, 0);
	F(p0, p7, pA, 0);
	F(p0, pA, pB, 0);

	F(p1, p5, p9, 1);
	F(p5, pB, p4, 1);
	F(pB, pA, p2, 1);
	F(pA, p7, p6, 1);
	F(p7, p1, p8, 1);

	F(p3, p9, p4, 2);
	F(p3, p4, p2, 2);
	F(p3, p2, p6, 2);
	F(p3, p6, p8, 2);
	F(p3, p8, p9, 2);

	F(p4, p9, p5, 3);
	F(p2, p4, pB, 3);
	F(p6, p2, pA, 3);
	F(p8, p6, p7, 3);
	F(p9, p8, p1, 3);

	std::swap(tmpt, tris_);
	tmpt.clear();
	// now, px/py/pz & tris contain the first level mesh

	// 2. subdivide divs# of times
	for (int dnum{0}; dnum<divs; ++dnum) {
		int numTris = static_cast<int>(tris_.size()) / 3;
		int newTris = numTris * 4;
		tmpt.reserve(newTris * 3);

		for (int i=0; i<int(tris_.size()); i+=4) {
			auto i0=tris_[i+0], i1=tris_[i+1], i2=tris_[i+2];
			auto color = tris_[i+3];

			auto i0i1 = MP(i0, i1, GP(i0), GP(i1));
			auto i1i2 = MP(i1, i2, GP(i1), GP(i2));
			auto i2i0 = MP(i2, i0, GP(i2), GP(i0));

			/*
			 *       i0
			 *     /  a \
			 *   i0i1--i2i0
			 *   /b \ c/ d\
			 *  i1--i1i2--i2
			 */

			F( i0,  i0i1, i2i0, color);   // a
			F(i0i1,  i1,  i1i2, color);   // b
			F(i0i1, i1i2, i2i0, color);   // c
			F(i2i0, i1i2,  i2 , color);}  // d

		std::swap(tmpt, tris_);
		tmpt.clear();
		edges_.clear(); }


	// 3. identify all points that touch reqColor'd faces
	std::vector<bool> pf(px.size(), false);
	for (int i=0; i<int(tris_.size()); i+=4) {
		auto i0=tris_[i+0], i1=tris_[i+1], i2=tris_[i+2];
		auto color = tris_[i+3];
		if (color == reqColor) {
			pf[i0] = true;
			pf[i1] = true;
			pf[i2] = true; }}

	// 4a. create indices list for the requested subgraph
	// copy colored tris into the final indices list,
	// put unwanted tris that share wanted vertices
	// into a temporary list (fringes)
	tmpt.clear();
	std::vector<int> fringe;
	for (int i=0; i<int(tris_.size()); i+=4) {
		auto i0=tris_[i+0], i1=tris_[i+1], i2=tris_[i+2];
		auto color = tris_[i+3];
		if (color == reqColor) {
			tmpt.emplace_back(i0);
			tmpt.emplace_back(i1);
			tmpt.emplace_back(i2); }
		else {
			if (pf[i0] || pf[i1] || pf[i2]) {
				fringe.emplace_back(i0);
				fringe.emplace_back(i1);
				fringe.emplace_back(i2); }}}
	swap(tris_, tmpt);

	// 4b. record the number of indices to use when rendering
	numRenderIndices_ = static_cast<int>(tris_.size());

	// 4c. append the fringe tris, for e.g. vertex normal calc
	tris_.insert(end(tris_), begin(fringe), end(fringe));

	// 5. finalize
	Optimize();
	Sphereize();
	vCnt_ = static_cast<int>(px.size());
	Pad(); }


/**
 * move all points to the unit sphere surface
 */
void Icosphere::Sphereize() {
	// sphereize
	for (int i=0; i<int(px.size()); ++i) {
		UP(i, normalize(GP(i))); }}


/**
 * pad point lists for sse reads
 */
void Icosphere::Pad() {
	int target = (static_cast<int>(px.size()) + 3) / 4;
	while (int(px.size()) < target) {
		px.emplace_back(0.0F);
		py.emplace_back(0.0F);
		pz.emplace_back(0.0F); }}
	

/**
 * optimize the point list to improve locality
 * probably naive, but should help
 */
void Icosphere::Optimize() {
	int seq=0;
	const int vcnt = static_cast<int>(px.size());
	std::vector<int> access(vcnt, -1);
	for (int i=0; i<int(tris_.size()); i+=3) {
		int i0=tris_[i], i1=tris_[i+1], i2=tris_[i+2];
		if (access[i0] == -1) access[i0] = seq++;
		if (access[i1] == -1) access[i1] = seq++;
		if (access[i2] == -1) access[i2] = seq++; }

	int newvcnt = seq;

	rcls::vector<float> tmp;
	tmp = px;
	px.resize(newvcnt);
	for (int i=0; i<vcnt; i++) {
		if (access[i] == -1) continue;
		px[access[i]] = tmp[i]; }
	tmp = py;
	py.resize(newvcnt);
	for (int i=0; i<vcnt; i++) {
		if (access[i] == -1) continue;
		py[access[i]] = tmp[i]; }
	tmp = pz;
	pz.resize(newvcnt);
	for (int i=0; i<vcnt; i++) {
		if (access[i] == -1) continue;
		pz[access[i]] = tmp[i]; }

	for (int i=0; i<int(tris_.size()); i++) {
		tris_[i] = access[tris_[i]]; }}


/**
 * add a point, return its index
 */
inline int Icosphere::AP(float x, float y, float z) {
	int idx = static_cast<int>(px.size());
	px.emplace_back(x);
	py.emplace_back(y);
	pz.emplace_back(z);
	return idx; }


/**
 * update an existing point
 */
inline void Icosphere::UP(int i, rmlv::vec3 d) {
	px[i] = d.x;
	py[i] = d.y;
	pz[i] = d.z; }


/**
 * (maybe) add a midpoint
 * important: midpoints are stored in an edge list
 *            so the same point will be used by
 *            both faces that share the same edge
 *            ensuring the new mesh is water-tight
 */
inline int Icosphere::MP(int i0, int i1, rmlv::vec3 p0, rmlv::vec3 p1) {
	if (i0 > i1) {
		std::swap(i0, i1); }
	uint64_t key = ((uint64_t)i0<<32)|i1;
	if (auto found = edges_.find(key); found != end(edges_)) {
		return found->second; }
	
	auto newP = mix(p0, p1, 0.5F);
	int newIdx = AP(newP.x, newP.y, newP.z);
	edges_[key] = newIdx;
	return newIdx; }


}  // namespace rglv
}  // namespace rqdq
