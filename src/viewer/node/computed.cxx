#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <memory>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_value.hxx"

#include "3rdparty/exprtk/exprtk.hpp"

namespace rqdq {
namespace {

using namespace rqv;
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
	IValue* sourceNode{nullptr};
	std::string sourceSlot{};
	std::vector<double> data{}; };


/**
 * per-thread state for Computed*Node
 */
struct ComputedNodeState {
	std::vector<ComputedInput> computedInputs;
	exprtk::symbol_table<double> symbolTable;
	exprtk::expression<double> expression;
	exprtk::parser<double> parser; };


/**
 * vec3 computed using exprTk, with inputs from other nodes
 */
class ComputedVec3Node : public IValue {
public:
	using VarDefList = std::vector<std::pair<std::string, std::string>>;

	ComputedVec3Node(std::string_view id, InputList inputs, std::string code, VarDefList varDefs)
		:IValue(id, std::move(inputs)) {

		// initialize a ComputedNodeState for each thread
		for (int threadNum=0; threadNum<jobsys::numThreads; threadNum++) {
			cache_pt_.emplace_back();

			auto td = std::make_unique<ComputedNodeState>();
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
			state_pt_.push_back(move(td)); }}

	// NodeBase
	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		bool connected = false;
		// apply the same connections for all threads
		for (auto& td : state_pt_) {
			for (auto& computed_input : td->computedInputs) {
				if (computed_input.name == attr) {
					computed_input.sourceNode = dynamic_cast<IValue*>(other);
					computed_input.sourceSlot = slot;
					if (computed_input.sourceNode == nullptr) {
						TYPE_ERROR(IValue);
						return false; }
					connected = true; }}}
		if (connected) {
			return true; }
		return IValue::Connect(attr, other, slot); }

	void Reset() override {
		for (auto& cache : cache_pt_) {
			cache.clear(); }}

	// IValue
	NamedValue Eval(std::string_view name) override {
		rmlv::vec3 result;
		auto& td = state_pt_[jobsys::threadId];
		auto& cache = cache_pt_[jobsys::threadId];
		auto& computedInputs = td->computedInputs;
		auto& expression = td->expression;

		thread_local std::string cacheKey;
		cacheKey.assign(name);
		auto search = cache.find(cacheKey);
		if (search != end(cache)) {
			return search->second; }

		// load inputs
		for (auto& ci : computedInputs) {
			if (ci.sourceNode != nullptr) {
				switch (ci.type) {
				case ValueType::Integer:
					{int value = ci.sourceNode->Eval(ci.sourceSlot).as_int();
					ci.data[0] = double(value); }
					break;
				case ValueType::Real:
					{float value = ci.sourceNode->Eval(ci.sourceSlot).as_float();
					ci.data[0] = double(value); }
					break;
				case ValueType::Vec2:
					{vec2 value = ci.sourceNode->Eval(ci.sourceSlot).as_vec2();
					ci.data[0] = double(value.x);
					ci.data[1] = double(value.y); }
					break;
				case ValueType::Vec3:
					{vec3 value = ci.sourceNode->Eval(ci.sourceSlot).as_vec3();
					ci.data[0] = double(value.x);
					ci.data[1] = double(value.y);
					ci.data[2] = double(value.z); }
					break;
				case ValueType::Vec4:
					{vec4 value = ci.sourceNode->Eval(ci.sourceSlot).as_vec4();
					ci.data[0] = double(value.x);
					ci.data[1] = double(value.y);
					ci.data[2] = double(value.z);
					ci.data[4] = double(value.w); }}}}

		// compute and gather results
		expression.value();
		if (expression.results().count() != 0u) {
			const auto& results = expression.results();
			const auto out = results[0];
			exprtk::igeneric_function<double>::generic_type::vector_view data(out);
			result = vec3{ float(data[0]), float(data[1]), float(data[2]) }; }
		else {
			result = vec3(0, 0, 0); }

		NamedValue out;
		if (name == "x") {
			out = NamedValue{ result.x }; }
		else if (name == "y") {
			out = NamedValue{ result.y }; }
		else if (name == "z") {
			out = NamedValue{ result.z }; }
		else if (name == "xy") {
			out = NamedValue{ result.xy() }; }
		else {
			out = NamedValue{ result }; };
		cache[cacheKey] = out;
		return out; }

private:
	std::vector<std::unique_ptr<ComputedNodeState>> state_pt_;
	std::vector<std::unordered_map<std::string, NamedValue>> cache_pt_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		using rclx::jv_find;
		std::string code{"var out[3] := {0, 0, 0}; return [out];"};
		std::vector<std::pair<std::string, std::string>> svars;

		if (auto jv = jv_find(data_, "code", JSON_STRING)) {
			code.assign(jv->toString());
			if (code[0] == '{') {
				code = "var out[3] := " + code + "; return [out];"; }}

		if (auto jv = jv_find(data_, "inputs", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				auto jv_name = jv_find(item->value, "name", JSON_STRING);
				auto jv_type = jv_find(item->value, "type", JSON_STRING);
				auto jv_source = jv_find(item->value, "source", JSON_STRING);
				if (jv_name && jv_type && jv_source) {
					svars.push_back(std::pair{jv_name->toString(), jv_type->toString()});
					inputs_.emplace_back(jv_name->toString(), jv_source->toString()); } } }

		out_ = std::make_shared<ComputedVec3Node>(id_, std::move(inputs_), std::move(code), std::move(svars)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$computedVec3", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
