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

#include "3rdparty/fmt/include/fmt/printf.h"
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


NodeRegistry& NodeRegistry::GetInstance() {
	static NodeRegistry reg{};
	return reg; }


void NodeRegistry::Register(NodeInfo info) {
	if (GetByTypeName(info.typeName).has_value()) {
		auto msg = fmt::sprintf("attempted to register node type "
								"\"%s\" twice", info.typeName);
		throw std::runtime_error(msg); }
	if (GetByJsonName(info.jsonName).has_value()) {
		auto msg = fmt::sprintf("attempted to register json object "
								"\"%s\" twice", info.jsonName);
		throw std::runtime_error(msg); }

	jsonDB_.emplace(info.jsonName, info);
	typeDB_.emplace(info.typeName, info); }


std::optional<NodeInfo> NodeRegistry::GetByTypeName(std::string_view typeName) {
	static std::string key;
	key.assign(typeName);  // XXX yuck!
	if (auto found = typeDB_.find(key); found != end(typeDB_)) {
		return found->second; }
	return {}; }


std::optional<NodeInfo> NodeRegistry::GetByJsonName(std::string_view jsonName) {
	static std::string key;
	key.assign(jsonName);  // XXX yuck!
	if (auto found = jsonDB_.find(key); found != end(jsonDB_)) {
		return found->second; }
	return {}; }


CompileResult CompileNode(JsonValue data, const rglv::MeshStore& meshStore);


NodeCompiler::NodeCompiler() = default;


NodeCompiler::~NodeCompiler() = default;


CompileResult NodeCompiler::Compile(std::string_view id, JsonValue data, const rglv::MeshStore& meshStore) {
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


bool NodeCompiler::Input(std::string_view typeName, std::string_view attrName, bool required) {
	auto maybeTypeInfo = NodeRegistry::GetInstance().GetByTypeName(typeName);
	if (!maybeTypeInfo.has_value()) {
		auto msg = fmt::sprintf("node type \"%s\" not registered", typeName);
		std::cerr << msg;
		return false; }

	if (auto jv = jv_find(data_, attrName)) {
		if (jv->getTag() == JSON_STRING) {
			// reference string
			inputs_.emplace_back(attrName, jv->toString());
			return true; }
		if (jv->getTag() == JSON_OBJECT) {
			// inline node
			if (auto subPtr = CompileNode(jv.value(), *meshStore_)) {
				auto [subNode, subDeps] = *subPtr;
				if (maybeTypeInfo.value().IsInstance(subNode.get())) {
					deps_.emplace_back(subNode);
					std::copy(begin(subDeps), end(subDeps), std::back_inserter(deps_));
					inputs_.emplace_back(attrName, subNode->get_id());
					return true; }
				else {
					std::cerr << "inline node is not a " << typeName << "\n";
					return false; }}
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


optional<tuple<NodeInfo, JsonValue, std::string>> IdentifyNode(JsonValue data) {
	auto& registry = NodeRegistry::GetInstance();
	if (data.getTag() == JSON_OBJECT) {
		// it's an object
		if (auto node = data.toNode(); node) {
			// with at least one node
			if (auto nodeInfo = registry.GetByJsonName(node->key); nodeInfo) {
				// the first node has a key with a valid node type
				if (node->value.getTag() == JSON_OBJECT) {
					// with an object as its value
					std::string guid;
					if (auto jv = jv_find(node->value, "id", JSON_STRING)) {
						// that object includes an id key with string value
						guid.assign(jv->toString()); }
					else {
						guid = fmt::sprintf("__auto%d__", idGen++); }
					return std::tuple{ nodeInfo.value(), node->value, guid }; }}
			else {
				std::cerr << "not registered: " << node->key << "\n"; }}}
	return {};}


CompileResult CompileNode(JsonValue data, const rglv::MeshStore& meshStore) {
	if (auto info = IdentifyNode(data)) {
		const auto[nodeInfo, data, guid] = *info;
		return nodeInfo.compiler->Compile(guid, data, meshStore); }
	return {}; }


tuple<bool, NodeList> CompileDocument(const JsonValue root, const rglv::MeshStore& meshStore) {
	using std::cerr;
	NodeList nodes;

	// add the json nodes
	bool success = true;
	// XXX root is assumed to be a json list
	for (const auto& jsonItem : root) {
		auto instance = CompileNode(jsonItem->value, meshStore);
		if (instance.has_value()) {
			auto [node, deps] =  *instance;
			nodes.push_back(node);
			std::copy(begin(deps), end(deps), std::back_inserter(nodes)); }
		else {
			cerr << "error deserializing json item\n";
			success = false; }}

	return tuple{success, nodes}; }


bool Link(NodeList& nodes) {
	using std::cerr;
	unordered_map<string, int> byId;

	// index by id, check for duplicates
	std::string nodeId;
	for (int idx=0; idx<nodes.size(); idx++) {
		// std::cerr << "  " << nodes[idx]->get_id() << "\n";
		nodeId.assign(nodes[idx]->get_id());  // xxx yuck
		if (auto existing = byId.find(nodeId); existing != end(byId)) {
			cerr << "error: node id \"" << nodeId << "\" not unique\n";
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
				node->Connect(destAttr, depNode, depSlot); } // XXX change order of args
			else {
				cerr << "error: node for ref " << depNodeRef << " not found\n";
				return false; }}}

	return true; }


}  // namespace rqv
}  // namespace rqdq
