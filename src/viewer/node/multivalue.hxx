#pragma once
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_value.hxx"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace rqdq {
namespace rqv {

class MultiValueNode : public IValue {

	std::unordered_map<std::string, NamedValue> db_{};
	//NamedValue notFoundValue_{ "__notfound__", int{0}}; };
	NamedValue notFoundValue_{ float{0}}; 

public:
	MultiValueNode(std::string_view id, InputList inputs) :
		IValue(id, std::move(inputs)) {}

	template<typename T>
	void Upsert(const std::string& name, const T value) {
		db_[name] = NamedValue{ value }; }

	auto Eval(std::string_view name) -> NamedValue override {
		thread_local std::string tmp;
		tmp.assign(name);  // xxx yuck
		auto search = db_.find(tmp);
		return search == end(db_) ? notFoundValue_ : search->second; }};


}  // namespace rqv
}  // namespace rqdq
