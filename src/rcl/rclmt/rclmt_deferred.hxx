#pragma once
#include <functional>

namespace rqdq {
namespace rclmt {

template <typename GoodT, typename BadT>
struct Deferred {
	using goodfunc = std::function<void(GoodT)>;
	using badfunc = std::function<void(BadT)>;

//	Deferred(const Deferred&) = delete;
//	Deferred& operator=(const Deferred&) = delete;

	void Callback(GoodT data) {
		if (callback) {
			callback(data); }}

	void Errback(BadT data) {
		if (errback) {
			errback(data); }}

	void AddCallbacks(goodfunc f1, badfunc f2) {
		callback = std::move(f1);
		errback = std::move(f2); }

	goodfunc callback;
	badfunc errback; };


}  // namespace rclmt
}  // namespace rqdq
