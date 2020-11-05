#include "compile.hxx"

#include <cassert>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/viewer/node/base.hxx"

#include <fmt/printf.h>
#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace {

int idGen{0};

}  // namespace

namespace rqv {

using namespace std::string_literals;
using namespace std;
using namespace rclx;
using namespace rmlv;
using rclt::Split;


NodeRegistry::NodeRegistry() = default;


auto NodeRegistry::GetInstance() -> NodeRegistry& {
	static NodeRegistry reg{};
	return reg; }


void NodeRegistry::Register(std::string_view jsonName, FactoryFunc factoryFunc) {
	if (Get(jsonName) != nullptr) {
		auto msg = fmt::sprintf("attempted to register json object "
								"\"%s\" twice", jsonName);
		throw std::runtime_error(msg); }
	db_.emplace(jsonName, factoryFunc); }


auto NodeRegistry::Get(std::string_view jsonName) -> FactoryFunc {
	static std::string key;
	key.assign(jsonName);  // XXX yuck!
	if (auto found = db_.find(key); found != end(db_)) {
		return found->second; }
	return {}; }


auto NodeRegistry::Get(const std::string& jsonName) -> FactoryFunc {
	if (auto found = db_.find(jsonName); found != end(db_)) {
		return found->second; }
	return {}; }


auto CompileNode(JsonValue data, const rglv::MeshStore& meshStore) -> CompileResult;


NodeCompiler::NodeCompiler() = default;


NodeCompiler::~NodeCompiler() = default;


auto NodeCompiler::Compile(std::string_view id, JsonValue data, const rglv::MeshStore& meshStore) -> CompileResult {
	id_ = id;
	data_ = data;
	meshStore_ = &meshStore;
	inputs_.clear();
	deps_.clear();
	out_.reset();

	Build();
	if (!out_) {
		return {}; }
	return std::tuple{out_, deps_}; }


auto NodeCompiler::Input(std::string_view attrName, bool required) -> bool {
	if (auto jv = jv_find(data_, attrName)) {
		if (jv->getTag() == JSON_STRING) {
			// reference string
			inputs_.emplace_back(attrName, jv->toString());
			return true; }
		if (jv->getTag() == JSON_OBJECT) {
			// inline node
			if (auto subPtr = CompileNode(jv.value(), *meshStore_)) {
				auto [subNode, subDeps] = *subPtr;
				deps_.emplace_back(subNode);
				deps_.insert(end(deps_), begin(subDeps), end(subDeps));
				inputs_.emplace_back(attrName, subNode->get_id());
				return true; }
			else {
				std::cerr << "invalid data for " << attrName << "\n";
				return false; }}
		else {
			std::cerr << attrName << " node must be object or string\n";
			return false;}}
	else {
		if (required) {
			std::cerr << "missing " << attrName << " node\n";
			return false; }
		else {
			return true; }}}


auto IdentifyNode(JsonValue data) -> optional<tuple<NodeRegistry::FactoryFunc, JsonValue, std::string>> {
	auto& registry = NodeRegistry::GetInstance();
	if (data.getTag() == JSON_OBJECT) {
		// it's an object
		if (auto node = data.toNode(); node) {
			// with at least one node
			if (auto compilerFactory = registry.Get(string_view{node->key}); compilerFactory) {
				// the first node has a key with a valid node type
				if (node->value.getTag() == JSON_OBJECT) {
					// with an object as its value
					std::string guid;
					if (auto jv = jv_find(node->value, "id", JSON_STRING)) {
						// that object includes an id key with string value
						guid.assign(jv->toString()); }
					else {
						guid = fmt::sprintf("__auto%d__", idGen++); }
					// std::cerr << "IdentifyNode id=\"" << guid << "\"\n";
					return std::tuple{ compilerFactory, node->value, guid }; }}
			else {
				std::cerr << "not registered: " << node->key << "\n"; }}}
	return {};}


auto CompileNode(JsonValue data, const rglv::MeshStore& meshStore) -> CompileResult {
	if (auto info = IdentifyNode(data)) {
		const auto[compilerFactory, innerData, guid] = *info;
		auto nodeCompiler = compilerFactory();
		return nodeCompiler->Compile(guid, innerData, meshStore); }
	return {}; }


auto CompileDocument(const JsonValue root, const rglv::MeshStore& meshStore) -> tuple<bool, NodeList> {
	NodeList nodes;

	// add the json nodes
	bool success = true;
	// XXX root is assumed to be a json list
	for (const auto& jsonItem : root) {
		auto instance = CompileNode(jsonItem->value, meshStore);
		if (instance.has_value()) {
			auto [node, deps] =  *instance;
			nodes.push_back(node);
			nodes.insert(end(nodes), begin(deps), end(deps)); }
		else {
			std::cerr << "error deserializing json item\n";
			success = false; }}

	return tuple{success, nodes}; }


auto Link(NodeList& nodes) -> bool {
	unordered_map<string, int> byId;

	// index by id, check for duplicates
	std::string nodeId;
	for (int idx=0; idx<int(nodes.size()); idx++) {
		// std::cerr << "  " << nodes[idx]->get_id() << "\n";
		nodeId.assign(nodes[idx]->get_id());  // xxx yuck
		if (auto existing = byId.find(nodeId); existing != end(byId)) {
			std::cerr << "error: node id \"" << nodeId << "\" not unique\n";
			return false; }
		byId[nodeId] = idx; }

	// resolve links, replace with pointers
	for (auto& node : nodes) {
		for (const auto& input : node->get_inputs()) {
			const auto&[destAttr, depNodeRef] = input;
			// cerr << "node(" << node->name << ") will get input \"" << destAttr << "\" from " << depNodeRef << endl;

			// deserialize reference
			string depId;
			string depSlot;
			auto parts = Split(depNodeRef, ':');
			if (parts.size() == 1) {
				depId = parts[0];  depSlot = "default"s; }
			else {
				depId = parts[0];  depSlot = parts[1]; }

			if (auto search = byId.find(depId); search != end(byId)) {
				auto depNode = nodes[search->second].get();
				auto success = node->Connect(destAttr, depNode, depSlot);  // XXX change order of args
				if (!success) {
					return false; }}
			else {
				cerr << "error: node for ref " << depNodeRef << " not found\n";
				return false; }}}

	return true; }


}  // namespace rqv
}  // namespace rqdq
