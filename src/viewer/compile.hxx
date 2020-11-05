#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/viewer/node/base.hxx"

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rqv {

using NodeList = std::vector<std::shared_ptr<NodeBase>>;

using CompileResult = std::optional<std::tuple<std::shared_ptr<NodeBase>, NodeList>>;



class NodeCompiler {
protected:
	std::string_view id_{};
	JsonValue data_;
	const rglv::MeshStore* meshStore_{nullptr};

	std::shared_ptr<NodeBase> out_{};
	InputList inputs_{};
	NodeList deps_{};

public:
	NodeCompiler();
	virtual ~NodeCompiler();

	auto Compile(std::string_view id, JsonValue data, const rglv::MeshStore& meshStore) -> CompileResult;


	virtual void Build() = 0;

protected:
	auto DataReal(const char* key, float dv) -> float {
		if (auto jv = rclx::jv_find(data_, key, JSON_NUMBER)) {
			return static_cast<float>(jv->toNumber()); }
		else { return dv; }}

	auto DataInt(const char* key, int dv) -> int {
		if (auto jv = rclx::jv_find(data_, key, JSON_NUMBER)) {
			return static_cast<int>(jv->toNumber()); }
		else { return dv; }}

	auto DataBool(const char* key, bool dv) -> bool {
		if (auto jv = rclx::jv_find(data_, key, JSON_TRUE)) { return true; }
		if (auto jv = rclx::jv_find(data_, key, JSON_FALSE)) { return false; }
		return dv; }

	auto DataString(const char* key, std::string dv) -> std::string {
		if (auto jv = rclx::jv_find(data_, key, JSON_STRING)) { return jv->toString(); }
		return dv; }

	auto Input(std::string_view attrName, bool required) -> bool; };


class NodeRegistry {
public:
	using FactoryFunc = std::function<std::unique_ptr<NodeCompiler>()>;

	static
	auto GetInstance() -> NodeRegistry&;

private:
	std::unordered_map<std::string, FactoryFunc> db_;

private:
	NodeRegistry();  // private ctor

public:
	void Register(std::string_view jsonName, FactoryFunc factoryFunc);
	auto Get(std::string_view jsonName) -> FactoryFunc;
	auto Get(const std::string& jsonName) -> FactoryFunc; };


auto CompileDocument(JsonValue root, const rglv::MeshStore& meshstore) -> std::tuple<bool, NodeList>;
auto Link(NodeList& nodes) -> bool;


}  // namespace rqv
}  // namespace rqdq
