/**
 * signals (runtime callback list)
 *
 * Similar to Wink-Signals, but signals are Disconnect()able
 * using an id (int) returned by Connect().
 *
 * See also:
 * Wink-Signals: https://github.com/miguelmartin75/Wink-Signals/
 * js-signals: https://millermedeiros.github.io/js-signals/
 */
#pragma once
#include <algorithm>
#include <functional>
#include <vector>

namespace rqdq {
namespace rclmt {

template <typename SIGNATURE>
class Signal {
public:
	using func_type = std::function<SIGNATURE>;

	void Clear() {
		slots_.clear(); }

	template <typename... Args>
	int Connect(Args&&... args) {
		int id = nextId_++;
		slots_.emplace_back(id, std::forward<Args>(args)...);
		return id; }

	void Disconnect(int id) {
		auto search = std::find_if(begin(slots_), end(slots_),
								   [=](auto& item) { return item.first == id; });
		if (search != end(slots_)) {
			slots_.erase(search); }}

	template <typename... Args>
	void Emit(Args&&... args) const {
		for (auto& slot : slots_) {
			slot.second(std::forward<Args>(args)...); }}

private:
	std::vector<std::pair<int, func_type>> slots_;
	int nextId_{0}; };

}  // namespace rclmt
}  // namespace rqdq
