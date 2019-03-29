#include "src/rml/rmls/rmls_bench.hxx"

#include <algorithm>
#include <iostream>
#include <vector>

namespace rqdq {

namespace {

template <typename T>
const int len(const T& container) {
	return int(container.size()); }

template <typename T>
T sorted(const T& container) {
	T out(container);
	sort(out.begin(), out.end());
	return out; }

}  // close unnamed namespace

namespace rmls {


struct BenchStat calc_stat(const std::vector<double>& _timings, const double discard) {
	auto timings = sorted(_timings);
	const auto entries_to_keep = int(len(timings) * (1.0 - discard));
	timings.resize(entries_to_keep);

	auto count = len(timings);

	auto _min = timings[0];
	auto _max = timings[count - 1];
	auto idx_25 = count / 4;
	auto _25th = timings[idx_25];
	auto idx_75 = idx_25 * 3;
	auto _med = timings[count / 2];
	auto _75th = timings[idx_75];

	double ax = 0;
	for (const auto &val : timings) {
		ax += val; }
	auto _mean = ax /= count;

	ax = 0;
	for (const auto &val : timings) {
		double tmp = val - _mean;
		ax += (tmp * tmp); }
	ax /= count;
	auto _sdev = sqrt(ax);

	return{ _min, _max, _25th, _med, _75th, _mean, _sdev }; }

}  // close package namespace
}  // close enterprise namespace
