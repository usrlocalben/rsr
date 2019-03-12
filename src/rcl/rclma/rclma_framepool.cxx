#include "src/rcl/rclma/rclma_framepool.hxx"

#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"

namespace rqdq {
namespace {

thread_local int thread_id;
int generation;
int total_jobs;
thread_local uint32_t pool_idx;
int thread_count;

std::vector<char*> pools;
int sps[128];

thread_local double mark_start_time;

int Ceil16(const int x) {
	int rag = x & 0xf;
	int segments = x >> 4;
	if (rag) {
		segments += 1; }
	return segments << 4; }


}  // namespace

namespace rclma {

namespace framepool {

void Init() {
	pools.clear();
	for (int ti = 0; ti < rclmt::jobsys::thread_count; ti++) {
		void * ptr = _aligned_malloc(100000 * 64, 64);
		pools.push_back(reinterpret_cast<char*>(ptr));
		sps[ti] = 0; }}


void Reset() {
	for (int ti = 0; ti < rclmt::jobsys::thread_count; ti++) {
		sps[ti] = 0; }}


void* Allocate(int amt) {
	amt = Ceil16(amt);
	auto my_store = pools[rclmt::jobsys::thread_id];
	auto& sp = sps[rclmt::jobsys::thread_id];
	void *out = reinterpret_cast<void*>(my_store + sp);
	sp += amt;
	return out; }


}  // namespace framepool


}  // namespace rclma
}  // namespace rqdq
