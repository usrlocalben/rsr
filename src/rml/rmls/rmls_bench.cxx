#include "src/rml/rmls/rmls_bench.hxx"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

namespace rqdq {
namespace rmls {

auto CalcStat(std::vector<double> samples, const double discardPct) -> BenchStat {
	std::sort(begin(samples), end(samples));
	const auto entries_to_keep = int(samples.size() * (1.0 - discardPct));
	samples.resize(entries_to_keep);

	auto cnt = samples.size();

	auto _min = samples.front();
	auto _max = samples.back();
	auto _p25 = samples[cnt*1/4];
	auto _med = samples[cnt*1/2];
	auto _p75 = samples[cnt*3/4];

	auto sum = std::accumulate(begin(samples), end(samples), 0.0);
	auto _avg = sum / samples.size();

	auto sqsum = accumulate(
		begin(samples), end(samples),
		0.0, 
		[=](double a, double b) {
			auto err = b - _avg;
			return a + (err*err); });

	auto _std = sqrt(sqsum / cnt);

	return{ _min, _max, _p25, _med, _p75, _avg, _std }; }

}  // close package namespace
}  // close enterprise namespace
