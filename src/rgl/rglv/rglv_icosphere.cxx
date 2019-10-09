#include "src/rgl/rglv/rglv_icosphere.hxx"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <unordered_map>

#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"


namespace rqdq {
namespace rglv {

Icosphere::Icosphere(int divs) {
	using rmlv::vec3;
	const float t = (1.0F + sqrt(5.0)) / 2.0F;

	const auto p0 = AP( -1.0F,  t, 0.0F );
	const auto p1 = AP(  1.0F,  t, 0.0F );
	const auto p2 = AP( -1.0F, -t, 0.0F );
	const auto p3 = AP(  1.0F, -t, 0.0F );

	const auto p4 = AP( 0.0F, -1,  t );
	const auto p5 = AP( 0.0F,  1,  t );
	const auto p6 = AP( 0.0F, -1, -t );
	const auto p7 = AP( 0.0F,  1, -t );

	const auto p8 = AP(    t, 0, -1 );
	const auto p9 = AP(    t, 0,  1 );
	const auto pA = AP(   -t, 0, -1 );
	const auto pB = AP(   -t, 0,  1 );

	tmps_.emplace();
	tmps_->reserve(5*4*3);

	F(p0, pB, p5);
	F(p0, p5, p1);
	F(p0, p1, p7);
	F(p0, p7, pA);
	F(p0, pA, pB);

	F(p1, p5, p9);
	F(p5, pB, p4);
	F(pB, pA, p2);
	F(pA, p7, p6);
	F(p7, p1, p8);

	F(p3, p9, p4);
	F(p3, p4, p2);
	F(p3, p2, p6);
	F(p3, p6, p8);
	F(p3, p8, p9);

	F(p4, p9, p5);
	F(p2, p4, pB);
	F(p6, p2, pA);
	F(p8, p6, p7);
	F(p9, p8, p1);

	std::swap(*tmps_, tris_);
	tmps_->clear();
	// px/py/pz & tris contain the first level mesh

	for (int dnum{0}; dnum<divs; ++dnum) {
		int numTris = tris_.size() / 3;
		int newTris = numTris * 4;
		tmps_->reserve(newTris * 3);

		for (int i=0; i<tris_.size(); i+=3) {
			auto i0=tris_[i+0], i1=tris_[i+1], i2=tris_[i+2];
			auto p0=GP(i0), p1=GP(i1), p2=GP(i2);

			auto i0i1 = MP(i0, i1, p0, p1);
			auto i1i2 = MP(i1, i2, p1, p2);
			auto i2i0 = MP(i2, i0, p2, p0);

			/*
			 *       i0
			 *     /  a \
			 *   i0i1--i2i0
			 *   /b \ c/ d\
			 *  i1--i1i2--i2
			 */

			F( i0,  i0i1, i2i0);   // a
			F(i0i1,  i1,  i1i2);   // b
			F(i0i1, i1i2, i2i0);   // c
			F(i2i0, i1i2,  i2 );}  // d

		std::swap(*tmps_, tris_);
		tmps_->clear();
		edges_.clear(); }

	vCnt_ = px.size();
	Sphereize();
	Optimize();
	Pad();
	tmps_ = {}; }


/**
 * move all points to the unit sphere surface
 */
void Icosphere::Sphereize() {
	// sphereize
	for (int i=0; i<px.size(); ++i) {
		UP(i, normalize(GP(i))); }}

/**
 * pad point lists for sse reads
 */
void Icosphere::Pad() {
	int target = (px.size() + 3) / 4;
	while (px.size() < target) {
		px.emplace_back(0);
		py.emplace_back(0);
		pz.emplace_back(0); }}
	

/**
 * optimize the point list to improve locality
 * probably naive, but should help
 */
void Icosphere::Optimize() {
	using std::cout;
	int seq=0;
	const int vcnt = px.size();
	std::vector<int> access(vcnt, -1);
	for (int i=0; i<tris_.size(); i+=3) {
		int i0=tris_[i], i1=tris_[i+1], i2=tris_[i+2];
		if (access[i0] == -1) access[i0] = seq++;
		if (access[i1] == -1) access[i1] = seq++;
		if (access[i2] == -1) access[i2] = seq++; }

	/*
	cout << "seq: " << seq << "\n";

	cout << "access = [";
	bool first = true;
	for (int i=0; i<vcnt; ++i) {
		if (first) {
			first = false; }
		else {
			cout << ", "; }
		cout << access[i]; }
	cout << "]\n";
	*/

	rcls::vector<float> tmp;
	tmp = px;
	for (int i=0; i<vcnt; i++) {
		px[access[i]] = tmp[i]; }
	tmp = py;
	for (int i=0; i<vcnt; i++) {
		py[access[i]] = tmp[i]; }
	tmp = pz;
	for (int i=0; i<vcnt; i++) {
		pz[access[i]] = tmp[i]; }

	for (int i=0; i<tris_.size(); i++) {
		tris_[i] = access[tris_[i]]; }}

// add point
inline int Icosphere::AP(float x, float y, float z) {
	int idx = px.size();
	px.emplace_back(x);
	py.emplace_back(y);
	pz.emplace_back(z);
	return idx; }

// update point
inline void Icosphere::UP(int i, rmlv::vec3 d) {
	px[i] = d.x;
	py[i] = d.y;
	pz[i] = d.z; }

// add midpoint
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

// add face to tmps
void Icosphere::F(int i0, int i1, int i2) {
	tmps_->emplace_back(i0);
	tmps_->emplace_back(i1);
	tmps_->emplace_back(i2); }


}  // namespace rglv
}  // namespace rqdq
