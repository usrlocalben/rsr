#include <rqv_node_computed.hxx>

#include <rclmt_jobsys.hxx>
#include <rmlv_vec.hxx>

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <exprtk.hpp>


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
	std::string name;
	ValueType type;
	ValuesBase *source_node;
	std::string source_slot;
	std::vector<double> data; };


/**
 * per-thread state for Computed*Node
 */
struct ComputedNodeState {
	vector<ComputedInput> computed_inputs;
	exprtk::symbol_table<double> symbol_table;
	exprtk::expression<double> expression;
	exprtk::parser<double> parser; };


ComputedVec3Node::ComputedVec3Node(
	const string& name,
	const InputList& inputs,
	const string& code,
	const vector<pair<string, string>>& input_definitions
	) :ValuesBase(name, inputs) {

	// initialize a ComputedNodeState for each thread
	for (int ti=0; ti<jobsys::thread_count; ti++) {
		auto td = make_unique<ComputedNodeState>();
		auto& computed_inputs = td->computed_inputs;
		auto& symbol_table = td->symbol_table;
		auto& expression = td->expression;
		auto& parser = td->parser;

		// exprtk's symbol_table will have pointers to
		// the values in this vector, so reallocation
		// is not allowed.  reserve in advance.
		computed_inputs.reserve(16);
		for (const auto& input_definition : input_definitions) {
			const auto&[input_name, input_type_name] = input_definition;
			const ValueType input_type = name_to_type(input_type_name);
			computed_inputs.push_back({ input_name, input_type, nullptr, "default" });
			auto& cii = computed_inputs.back();
			if (input_type == ValueType::Integer) {
				cii.data.resize(1);
				symbol_table.add_variable(input_name, cii.data[0]); }
			else if (input_type == ValueType::Real) {
				cii.data.resize(1);
				symbol_table.add_variable(input_name, cii.data[0]); }
			else if (input_type == ValueType::Vec2) {
				cii.data.resize(2);
				symbol_table.add_vector(input_name, cii.data); }
			else if (input_type == ValueType::Vec3) {
				cii.data.resize(3);
				symbol_table.add_vector(input_name, cii.data); }
			else if (input_type == ValueType::Vec4) {
				cii.data.resize(4);
				symbol_table.add_vector(input_name, cii.data); }}
		expression.register_symbol_table(symbol_table);
		parser.compile(code, expression);
		cn_thread_state.push_back(move(td)); }}


ComputedVec3Node::~ComputedVec3Node() = default;

void ComputedVec3Node::connect(const string& attr, NodeBase* node, const string& slot) {
	bool connected = false;

	// apply the same connections for all threads
	for (auto& td : cn_thread_state) {
		for (auto& computed_input : td->computed_inputs) {
			if (computed_input.name == attr) {
				computed_input.source_node = static_cast<ValuesBase*>(node);
				computed_input.source_slot = slot;
				connected = true; }}}
	if (connected) return;
	cout << "ComputedVec3Node(" << name << ") attempted to add " << node->name << ":" << slot << " as " << attr << endl; }


NamedValue ComputedVec3Node::get(const string& name) {
	rmlv::vec3 result;
	auto& td = cn_thread_state[jobsys::thread_id];
	auto& computed_inputs = td->computed_inputs;
	auto& expression = td->expression;

	// load inputs
	for (auto& computed_input : computed_inputs) {
		if (computed_input.source_node == nullptr) {
			continue; }
		if (computed_input.type == ValueType::Integer) {
			int value = computed_input.source_node->get(computed_input.source_slot).as_int();
			computed_input.data[0] = double(value); }
		else if (computed_input.type == ValueType::Real) {
			float value = computed_input.source_node->get(computed_input.source_slot).as_float();
			computed_input.data[0] = double(value); }
		else if (computed_input.type == ValueType::Vec2) {
			vec2 value = computed_input.source_node->get(computed_input.source_slot).as_vec2();
			computed_input.data[0] = double(value.x);
			computed_input.data[1] = double(value.y); }
		else if (computed_input.type == ValueType::Vec3) {
			vec3 value = computed_input.source_node->get(computed_input.source_slot).as_vec3();
			computed_input.data[0] = double(value.x);
			computed_input.data[1] = double(value.y);
			computed_input.data[2] = double(value.z); }
		else if (computed_input.type == ValueType::Vec4) {
			vec4 value = computed_input.source_node->get(computed_input.source_slot).as_vec4();
			computed_input.data[0] = double(value.x);
			computed_input.data[1] = double(value.y);
			computed_input.data[2] = double(value.z);
			computed_input.data[4] = double(value.w); }}

	// compute and gather results
	expression.value();
	if (expression.results().count()) {
		const auto results = expression.results();
		const auto out = results[0];
		exprtk::igeneric_function<double>::generic_type::vector_view data(out);
		result = vec3{ float(data[0]), float(data[1]), float(data[2]) }; }
	else {
		result = vec3(0, 0, 0); }

	if (name == "x") return NamedValue{ "x", result.x };
	if (name == "y") return NamedValue{ "y", result.y };
	if (name == "z") return NamedValue{ "z", result.z };
	if (name == "xy") return NamedValue{ "xy", result.xy() };
	return NamedValue{ "", result }; }


}  // close package namespace
}  // close enterprise namespace
