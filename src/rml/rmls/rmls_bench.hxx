#pragma once
#include <vector>

namespace rqdq {
namespace rmls {

struct BenchStat {
	double min;
	double max;
	double p25;
	double med;
	double p75;
	double avg;
	double std; };

auto CalcStat(std::vector<double> samples, double discard) -> BenchStat;


}  // close package namespace
}  // close enterprise namespace
