#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/viewer/node/base.hxx"

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace rqv {

using NodeList = std::vector<std::shared_ptr<NodeBase>>;

using CompileResult = std::optional<std::tuple<std::shared_ptr<NodeBase>, NodeList>>;

std::tuple<bool, NodeList> CompileDocument(JsonValue root, const rglv::MeshStore& meshstore);


class NodeCompiler {
public:
	NodeCompiler();
	virtual ~NodeCompiler();

	CompileResult Compile(std::string_view id, JsonValue data, const rglv::MeshStore& meshStore);

	virtual void Build() = 0;

protected:
	bool Input(std::string_view attrName, bool required);

	std::string_view id_{};
	JsonValue data_;
	const rglv::MeshStore* meshStore_{nullptr};

	std::shared_ptr<NodeBase> out_{};
	InputList inputs_{};
	NodeList deps_{}; };


class NodeRegistry {
private:
	NodeRegistry();
public:
	static NodeRegistry& GetInstance();
	void Register(std::string_view jsonName, NodeCompiler* compiler);
	NodeCompiler* Get(std::string_view jsonName);
	NodeCompiler* Get(const std::string& jsonName);

private:
	std::unordered_map<std::string, NodeCompiler*> db_; };


bool Link(NodeList& nodes);


}  // namespace rqv
}  // namespace rqdq
