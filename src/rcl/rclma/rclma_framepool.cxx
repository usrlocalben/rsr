#include "src/rcl/rclma/rclma_framepool.hxx"

#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"

namespace rqdq {
namespace {

std::vector<char*> pools;
std::vector<int> sp;


auto Ceil16(const int x) -> int {
	int rag = x & 0xf;
	int segments = x >> 4;
	if (rag != 0) {
		segments += 1; }
	return segments << 4; }


}  // namespace

namespace rclma {

namespace framepool {

void Init() {
	pools.clear();
	for (int ti=0; ti<rclmt::jobsys::numThreads; ++ti) {
		void * const buf = _aligned_malloc(100000 * 64, 64);
		pools.push_back(reinterpret_cast<char*>(buf)); }
	sp.resize( rclmt::jobsys::numThreads * 16, 0 ); }


void Reset() {
	for (int ti=0; ti<rclmt::jobsys::numThreads; ++ti) {
		sp[ti*16] = 0; }}


auto Allocate(int amt) -> void* {
	amt = Ceil16(amt);
	auto my_store = pools[rclmt::jobsys::threadId];
	auto& my_idx = sp[rclmt::jobsys::threadId * 16];
	void * const out = reinterpret_cast<void*>(my_store + my_idx);
	my_idx += amt;
	return out; }


}  // namespace framepool


}  // namespace rclma
}  // namespace rqdq
