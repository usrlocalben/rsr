#pragma once
#include <algorithm>

namespace rqdq {
namespace rclr {

template<typename Range, typename Function>
auto for_each(Range& range, Function f) {
	return std::for_each(begin(range), end(range), f); }

template<typename Range, typename Function>
auto find_if(Range& range, Function f) {
	return std::find_if(begin(range), end(range), f); }

template<typename Range, typename Function>
auto sort(Range& range, Function f) {
	return std::sort(begin(range), end(range), f); }

template<typename RangeA, typename RangeB>
auto copy(RangeA& source, RangeB& dest) {
	return std::copy(begin(source), end(source), begin(dest)); }

template <typename T, typename FUNC>
void each(const T& container, FUNC func) {
	int idx = 0;
	for (const auto& item : container) {
		func(item, idx);
		idx += 1; }}

template <typename T>
int len(const T& container) {
	return int(container.size()); }

}  // namespace rclr
}  // namespace rqdq
