#include "computed.hxx"

#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/node/value.hxx"

#include "3rdparty/exprtk/exprtk.hpp"

namespace rqdq {
namespace rqv {

using namespace std;
namespace jobsys = rclmt::jobsys;
using vec2 = rmlv::vec2;
using vec3 = rmlv::vec3;
using vec4 = rmlv::vec4;

/**
 * an input value slot for a ComputedNode
 */
struct ComputedInput {
	std::string name{};
	ValueType type{ValueType::Real};
	ValuesBase* sourceNode{nullptr};
	std::string sourceSlot{};
	std::vector<double> data{}; };


/**
 * per-thread state for Computed*Node
 */
struct ComputedNodeState {
	vector<ComputedInput> computedInputs;
	exprtk::symbol_table<double> symbolTable;
	exprtk::expression<double> expression;
	exprtk::parser<double> parser; };


ComputedVec3Node::ComputedVec3Node(
	std::string_view id,
	InputList inputs,
	std::string code,
	vector<pair<string, string>> varDefs)
	:ValuesBase(id, std::move(inputs)) {

	// initialize a ComputedNodeState for each thread
	for (int threadNum=0; threadNum<jobsys::thread_count; threadNum++) {
		auto td = make_unique<ComputedNodeState>();
		auto& computedInputs = td->computedInputs;
		auto& symbolTable = td->symbolTable;
		auto& expression = td->expression;
		auto& parser = td->parser;

		// exprtk's symbol_table will have pointers to
		// the values in this vector, so reallocation
		// is not allowed.  reserve in advance.
		computedInputs.reserve(16);
		for (const auto& varDef : varDefs) {
			const auto&[inputName, inputTypeName] = varDef;
			const ValueType inputType = ValueTypeSerializer::Deserialize(inputTypeName);
			computedInputs.emplace_back(ComputedInput{ inputName, inputType, nullptr, "default" });
			auto& cii = computedInputs.back();
			switch (inputType) {
			case ValueType::Integer:
				cii.data.resize(1);
				symbolTable.add_variable(inputName, cii.data[0]);
				break;
			case ValueType::Real:
				cii.data.resize(1);
				symbolTable.add_variable(inputName, cii.data[0]);
				break;
			case ValueType::Vec2:
				cii.data.resize(2);
				symbolTable.add_vector(inputName, cii.data);
				break;
			case ValueType::Vec3:
				cii.data.resize(3);
				symbolTable.add_vector(inputName, cii.data);
				break;
			case ValueType::Vec4:
				cii.data.resize(4);
				symbolTable.add_vector(inputName, cii.data);
				break; }}
		expression.register_symbol_table(symbolTable);
		parser.compile(code, expression);
		state_.push_back(move(td)); }}


ComputedVec3Node::~ComputedVec3Node() = default;

void ComputedVec3Node::Connect(std::string_view attr, NodeBase* other, string_view slot) {
	bool connected = false;

	// apply the same connections for all threads
	for (auto& td : state_) {
		for (auto& computed_input : td->computedInputs) {
			if (computed_input.name == attr) {
				computed_input.sourceNode = static_cast<ValuesBase*>(other);
				computed_input.sourceSlot = slot;
				connected = true; }}}
	if (connected) return;
	ValuesBase::Connect(attr, other, slot); }


NamedValue ComputedVec3Node::Get(string_view name) {
	rmlv::vec3 result;
	auto& td = state_[jobsys::thread_id];
	auto& computedInputs = td->computedInputs;
	auto& expression = td->expression;

	// load inputs
	for (auto& ci : computedInputs) {
		if (ci.sourceNode != nullptr) {
			switch (ci.type) {
			case ValueType::Integer:
				{int value = ci.sourceNode->Get(ci.sourceSlot).as_int();
				ci.data[0] = double(value); }
				break;
			case ValueType::Real:
				{float value = ci.sourceNode->Get(ci.sourceSlot).as_float();
				ci.data[0] = double(value); }
				break;
			case ValueType::Vec2:
				{vec2 value = ci.sourceNode->Get(ci.sourceSlot).as_vec2();
				ci.data[0] = double(value.x);
				ci.data[1] = double(value.y); }
				break;
			case ValueType::Vec3:
				{vec3 value = ci.sourceNode->Get(ci.sourceSlot).as_vec3();
				ci.data[0] = double(value.x);
				ci.data[1] = double(value.y);
				ci.data[2] = double(value.z); }
				break;
			case ValueType::Vec4:
				{vec4 value = ci.sourceNode->Get(ci.sourceSlot).as_vec4();
				ci.data[0] = double(value.x);
				ci.data[1] = double(value.y);
				ci.data[2] = double(value.z);
				ci.data[4] = double(value.w); }}}}

	// compute and gather results
	expression.value();
	if (expression.results().count()) {
		const auto results = expression.results();
		const auto out = results[0];
		exprtk::igeneric_function<double>::generic_type::vector_view data(out);
		result = vec3{ float(data[0]), float(data[1]), float(data[2]) }; }
	else {
		result = vec3(0, 0, 0); }

	if (name == "x") {
		return NamedValue{ "x", result.x }; }
	if (name == "y") {
		return NamedValue{ "y", result.y }; }
	if (name == "z") {
		return NamedValue{ "z", result.z }; }
	if (name == "xy") {
		return NamedValue{ "xy", result.xy() }; }
	return NamedValue{ "", result }; }


}  // namespace rqv
}  // namespace rqdq
