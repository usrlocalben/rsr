#include "src/viewer/rqv_compile.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rgl/rglr/rglr_texture.hxx"
#include "src/rgl/rglr/rglr_texture_load.hxx"
#include "src/viewer/rqv_node_camera.hxx"
#include "src/viewer/rqv_node_computed.hxx"
#include "src/viewer/rqv_node_fx_auraforlaura.hxx"
#include "src/viewer/rqv_node_fx_carpet.hxx"
#include "src/viewer/rqv_node_fx_foo.hxx"
#include "src/viewer/rqv_node_fx_mc.hxx"
#include "src/viewer/rqv_node_fx_xyquad.hxx"
#include "src/viewer/rqv_node_gl.hxx"
#include "src/viewer/rqv_node_material.hxx"
#include "src/viewer/rqv_node_gpu.hxx"
#include "src/viewer/rqv_node_render.hxx"
#include "src/viewer/rqv_node_translate.hxx"
#include "src/viewer/rqv_node_value.hxx"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "3rdparty/gason/gason.h"

namespace rqdq {
namespace {

int idGen = 0;

}  // close anonymous namespace
namespace rqv {

using namespace std::string_literals;
using namespace std;
using namespace rclx;
using namespace rmlv;
using rclt::explode;

using NodeList = std::vector<std::shared_ptr<NodeBase>>;

using CompileResult = std::optional<std::tuple<std::shared_ptr<NodeBase>, NodeList>>;
using CompileFunc = CompileResult(*)(const string&, const JsonValue&, const rglv::MeshStore&);


CompileResult deserializeNode(const JsonValue& data, const rglv::MeshStore& meshstore);


#define Required(NODETYPE, ATTRNAME) \
	if (auto jv = jv_find(data, #ATTRNAME )) { \
		if (jv->getTag() == JSON_STRING) { \
			inputs.emplace_back( #ATTRNAME , jv->toString()); } \
		else if (jv->getTag() == JSON_OBJECT) { \
			if (auto subPtr = deserializeNode(*jv, meshStore)) { \
				auto [subNode, subDeps] = *subPtr; \
				if (auto depNode = dynamic_cast< NODETYPE *>(subNode.get())) { \
					deps.push_back(subNode); \
					std::copy(subDeps.begin(), subDeps.end(), std::back_inserter(deps)); \
					inputs.emplace_back(#ATTRNAME, depNode->name); } \
				else { \
					std::cout << "inline node is not a " << #NODETYPE << "\n"; \
					return {}; }} \
			else { \
				std::cout << "invalid data for " << #ATTRNAME << "\n"; \
				return {}; }} \
		else { \
			std::cout << #ATTRNAME << " node must be object or string\n"; \
			return {};}} \
	else { \
		std::cout << "missing " << #ATTRNAME << " node\n"; \
		return {}; }

#define Optional(NODETYPE, ATTRNAME) \
	if (auto jv = jv_find(data, #ATTRNAME )) { \
		if (jv->getTag() == JSON_STRING) { \
			inputs.emplace_back( #ATTRNAME , jv->toString()); } \
		else if (jv->getTag() == JSON_OBJECT) { \
			if (auto subPtr = deserializeNode(*jv, meshStore)) { \
				auto [subNode, subDeps] = *subPtr; \
				if (auto depNode = dynamic_cast< NODETYPE *>(subNode.get())) { \
					deps.push_back(subNode); \
					std::copy(subDeps.begin(), subDeps.end(), std::back_inserter(deps)); \
					inputs.emplace_back(#ATTRNAME, depNode->name); } \
				else { \
					std::cout << "inline node is not a " << #NODETYPE << "\n"; \
					return {}; }} \
			else { \
				std::cout << "invalid data for " << #ATTRNAME << "\n"; \
				return {}; }} \
		else { \
			std::cout << #ATTRNAME << " node must be object or string\n"; \
			return {};}}


CompileResult compileFxAuraForLaura(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;

	Required(MaterialNode, material)
	Required(ValuesBase, freq)
	Required(ValuesBase, phase)

	string meshPath{"notfound.obj"};
	if (auto jv = jv_find(data, "mesh", JSON_STRING)) {
		meshPath = jv->toString(); }
	const auto& mesh = meshStore.get(meshPath);

	return tuple{make_shared<FxAuraForLaura>(id, inputs, mesh), deps}; }


CompileResult compileFxCarpet(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;

	Required(MaterialNode, material)
	Required(ValuesBase, freq)
	Required(ValuesBase, phase)

	return tuple{make_shared<FxCarpet>(id, inputs), deps}; }


CompileResult compilePerspective(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Required(ValuesBase, position)

	float ha = 3.14f;
	if (auto jv = jv_find(data, "h", JSON_NUMBER)) {
		ha = float(jv->toNumber()); }

	float va = 0.0f;
	if (auto jv = jv_find(data, "v", JSON_NUMBER)) {
		va = float(jv->toNumber()); }

	float fov = 45.0f;
	if (auto jv = jv_find(data, "fov", JSON_NUMBER)) {
		fov = float(jv->toNumber()); }

	auto origin = rmlv::vec2{0.0f};
	if (auto jv = jv_find(data, "originX", JSON_NUMBER)) {
		origin.x = float(jv->toNumber()); }
	if (auto jv = jv_find(data, "originY", JSON_NUMBER)) {
		origin.y = float(jv->toNumber()); }
	return tuple{make_shared<ManCamNode>(id, inputs, ha, va, fov, origin), deps}; }


CompileResult compileOrthographic(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	return tuple{make_shared<OrthographicNode>(id, inputs), deps}; }


CompileResult compileComputedVec3(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;

	string code = "var out[3] := {0, 0, 0}; return [out];";
	vector<pair<string, string>> svars;

	if (auto jv = jv_find(data, "code", JSON_STRING)) {
		code = jv->toString();
		if (code[0] == '{') {
			code = "var out[3] := " + code + "; return [out];"; } }

	if (auto jv = jv_find(data, "inputs", JSON_ARRAY)) {
		for (const auto& item : *jv) {
			auto jv_name = jv_find(item->value, "name", JSON_STRING);
			auto jv_type = jv_find(item->value, "type", JSON_STRING);
			auto jv_source = jv_find(item->value, "source", JSON_STRING);
			if (jv_name && jv_type && jv_source) {
				svars.push_back(std::pair{jv_name->toString(), jv_type->toString()});
				inputs.emplace_back(jv_name->toString(), jv_source->toString()); } } }

	return tuple{make_shared<ComputedVec3Node>(id, inputs, code, svars), deps}; }


CompileResult compileFxFoo(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Required(MaterialNode, material)

	string name{"notfound.obj"};
	if (auto jv = jv_find(data, "name", JSON_STRING)) {
		name = jv->toString(); }
	const auto& mesh = meshStore.get(name);

	return tuple{make_shared<FxFoo>(id, inputs, mesh), deps}; }


CompileResult compileFxMC(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Required(MaterialNode, material)
	Required(ValuesBase, frob)

	int precision = 32;
	if (auto jv = jv_find(data, "precision", JSON_NUMBER)) {
		precision = int(jv->toNumber()); }
	switch (precision) {
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
		break;
	default:
		std::cout << "FxMC: precision " << precision << " must be one of (8, 16, 32, 64, 128)!  precision will be 32\n";
		precision = 32; }

	int forkDepth = 2;
	if (auto jv = jv_find(data, "forkDepth", JSON_NUMBER)) {
		forkDepth = int(jv->toNumber()); }
	if (forkDepth < 0 || forkDepth > 4) {
		std::cout << "FxMC: forkDepth " << forkDepth << " exceeds limit (0 <= n <= 4)!  forkDepth will be 2\n";
		forkDepth = 2; }

	float range = 5.0f;
	if (auto jv = jv_find(data, "range", JSON_NUMBER)) {
		range = float(jv->toNumber()); }
	if (range < 0.0001f) {
		std::cout << "FxMC: range " << range << " is too small (n<0.0001)!  range will be +/- 5.0\n";
		range = 5.0f; }

	return tuple{make_shared<FxMC>(id, inputs, precision, forkDepth, range), deps}; }


CompileResult compileFxXYQuad(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	ShaderProgramId program = ShaderProgramId::Default;

	vec2 leftTop{ -1.0f, 1.0f };
	vec2 rightBottom{ 1.0f, -1.0f };
	float z = 0.0f;

	Required(MaterialNode, material)
	Required(ValuesBase, leftTop)
	Required(ValuesBase, rightBottom)
	Required(ValuesBase, z)

	return make_pair(make_shared<FxXYQuad>(id, inputs), deps); };


CompileResult compileMaterial(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Optional(TextureNode, texture0)
	Optional(TextureNode, texture1)
	Optional(ValuesBase, u0)
	Optional(ValuesBase, u1)

	ShaderProgramId program = ShaderProgramId::Default;
	if (auto jv = jv_find(data, "program", JSON_STRING)) {
		program = deserialize_program_name(jv->toString()); }

	bool filter = false;
	if (auto jv = jv_find(data, "filter", JSON_TRUE)) {
		filter = true; }

	return tuple{make_shared<MaterialNode>(id, inputs, program, filter), deps}; }


CompileResult compileImage(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;

	rglr::Texture tex;
	if (auto jv = jv_find(data, "file", JSON_STRING)) {
		tex = rglr::load_png(jv->toString(), jv->toString(), false);
		tex.maybe_make_mipmap(); }

	return tuple{make_shared<ImageNode>(id, inputs, tex), deps}; }


CompileResult compileGPU(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	if (auto jv = jv_find(data, "layers", JSON_ARRAY)) {
		for (const auto& item : *jv) {
			if (item->value.getTag() == JSON_STRING) {
				inputs.emplace_back("layer", item->value.toString()); }}}

	return tuple{make_shared<GPUNode>(id, inputs), deps}; }


CompileResult compileLayer(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Optional(CameraNode, camera)
	Optional(ValuesBase, color)
	if (auto jv = jv_find(data, "gl", JSON_ARRAY)) {
		for (const auto& item : *jv) {
			if (item->value.getTag() == JSON_STRING) {
				inputs.emplace_back("gl", item->value.toString()); } } }
	return tuple{make_shared<LayerNode>(id, inputs), deps}; }


CompileResult compileGroup(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	if (auto jv = jv_find(data, "gl", JSON_ARRAY)) {
		for (const auto& item : *jv) {
			if (item->value.getTag() == JSON_STRING) {
				inputs.emplace_back("gl", item->value.toString()); } } }
	return tuple{make_shared<GroupNode>(id, inputs), deps}; }


CompileResult compileLayerChooser(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Required(ValuesBase, selector)
	if (auto jv = jv_find(data, "layers", JSON_ARRAY)) {
		for (const auto& item : *jv) {
			if (item->value.getTag() == JSON_STRING) {
				inputs.emplace_back("layer", item->value.toString()); } } }
	return tuple{make_shared<LayerChooser>(id, inputs), deps}; }


CompileResult compileRender(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	Required(GPUNode, gpu)

	ShaderProgramId program = ShaderProgramId::Default;
	if (auto jv = jv_find(data, "program", JSON_STRING)) {
		program = deserialize_program_name(jv->toString()); }

	bool srgb = true;
	if (auto jv = jv_find(data, "sRGB", JSON_FALSE)) {
		srgb = false; }

	return tuple{make_shared<RenderNode>(id, inputs, program, srgb), deps}; }


CompileResult compileRenderToTexture(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;

	Required(GPUNode, gpu)

	int width = 256;
	if (auto jv = jv_find(data, "width", JSON_NUMBER)) {
		width = int(jv->toNumber()); }

	int height = 256;
	if (auto jv = jv_find(data, "height", JSON_NUMBER)) {
		height = int(jv->toNumber()); }

	bool aa = false;
	if (auto jv = jv_find(data, "aa", JSON_TRUE)) {
		aa = true; }

	float pa = 1.0f;
	if (auto jv = jv_find(data, "aspect", JSON_NUMBER)) {
		pa = float(jv->toNumber()); }

	return tuple{make_shared<RenderToTexture>(id, inputs, width, height, pa, aa), deps}; }


CompileResult compileTranslate(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	map<string, vec3> slot_values = {
		{"translate", vec3{0,0,0}},
		{"rotate", vec3{0,0,0}},
		{"scale", vec3{1,1,1}},
		};

	Optional(ValuesBase, rotate)
	Optional(ValuesBase, scale)
	Optional(ValuesBase, translate)
	Required(GlNode, gl)

	/*
	for (const auto& slot_name : { "translate", "rotate", "scale" }) {
		if (auto jv = jv_find(data, slot_name)) {
			auto value = jv_decode_ref_or_vec3(*jv);
			if (auto ptr = get_if<string>(&value)) {
				inputs.emplace_back(slot_name, *ptr); }
			else if (auto ptr = get_if<vec3>(&value)) {
				slot_values[slot_name] = *ptr; } } }

	if (auto jv = jv_find(data, "gl", JSON_STRING)) {
		inputs.emplace_back("gl", jv->toString()); }
		*/
	return tuple{make_shared<TranslateOp>(id, inputs), deps}; }


CompileResult compileRepeat(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	int many{ 1 };
	map<string, vec3> slot_values = {
		{"translate", vec3{0,0,0}},
		{"rotate", vec3{0,0,0}},
		{"scale", vec3{1,1,1}},
		};
	for (const auto& slot_name : { "translate", "rotate", "scale" }) {
		if (auto jv = jv_find(data, slot_name)) {
			auto value = jv_decode_ref_or_vec3(*jv);
			if (auto ptr = get_if<string>(&value)) {
				inputs.emplace_back(slot_name, *ptr); }
			else if (auto ptr = get_if<vec3>(&value)) {
				slot_values[slot_name] = *ptr; } } }
	if (auto jv = jv_find(data, "many", JSON_NUMBER)) {
		many = int(jv->toNumber()); }
	if (auto jv = jv_find(data, "gl", JSON_STRING)) {
		inputs.emplace_back("gl", jv->toString()); }
	return tuple{make_shared<RepeatOp>(
		id,
		inputs,
		many,
		slot_values["translate"],
		slot_values["rotate"],
		slot_values["scale"]
		), deps}; }


CompileResult compileFloat(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	InputList inputs;
	NodeList deps;
	float x{ 0.0f };

	if (auto jv = jv_find(data, "x", JSON_NUMBER)) {
		x = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "x", JSON_STRING)) {
		inputs.emplace_back("x", jv->toString()); }

	return tuple{make_shared<FloatNode>(id, inputs, x), deps}; }


CompileResult compileVec2(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	using rclx::jv_find;

	InputList inputs;
	NodeList deps;
	float x{ 0.0f }, y{ 0.0f };

	if (auto jv = jv_find(data, "x", JSON_NUMBER)) {
		x = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "x", JSON_STRING)) {
		inputs.emplace_back("x", jv->toString()); }

	if (auto jv = jv_find(data, "y", JSON_NUMBER)) {
		y = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "y", JSON_STRING)) {
		inputs.emplace_back("y", jv->toString()); }

	return tuple{make_shared<Vec2Node>(id, inputs, vec2{x, y}), deps}; }


CompileResult compileVec3(const string& id, const JsonValue& data, const rglv::MeshStore& meshStore) {
	using rclx::jv_find;

	InputList inputs;
	NodeList deps;
	float x{ 0.0f }, y{ 0.0f }, z{ 0.0f };

	if (auto jv = jv_find(data, "x", JSON_NUMBER)) {
		x = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "x", JSON_STRING)) {
		inputs.emplace_back("x", jv->toString()); }

	if (auto jv = jv_find(data, "y", JSON_NUMBER)) {
		y = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "y", JSON_STRING)) {
		inputs.emplace_back("y", jv->toString()); }

	if (auto jv = jv_find(data, "z", JSON_NUMBER)) {
		z = float(jv->toNumber()); }
	else if (auto jv = jv_find(data, "z", JSON_STRING)) {
		inputs.emplace_back("z", jv->toString()); }

	return tuple{make_shared<Vec3Node>(id, inputs, vec3{x, y, z}), deps}; }


enum class NodeType {
	// value nodes
	ComputedVec3,
	Float,
	Vec2,
	Vec3,

	// util nodes
	GPU,
	Perspective,
	Orthographic,
	Render,
	RenderToTexture,

	// gl nodes
	FxAuraForLaura,
	FxCarpet,
	FxFoo,
	FxMC,
	FxXYQuad,
	Group,
	Image,
	Layer,
	LayerChooser,
	Material,
	Repeat,
	Translate,
};


struct NodeMeta {
	//string jsonKey;
	NodeType nodeType;
	CompileFunc compileFunc; };


unordered_map<string, NodeMeta> nodeTable{
	{"$computedVec3"s,    NodeMeta{NodeType::ComputedVec3,    compileComputedVec3}},
	{"$float"s,           NodeMeta{NodeType::Float,           compileFloat}},
	{"$vec2"s,            NodeMeta{NodeType::Vec2,            compileVec2}},
	{"$vec3"s,            NodeMeta{NodeType::Vec3,            compileVec3}},
	{"$vec3"s,            NodeMeta{NodeType::Vec3,            compileVec3}},

	{"$gpu"s,             NodeMeta{NodeType::GPU,             compileGPU}},

	{"$perspective"s,     NodeMeta{NodeType::Perspective,     compilePerspective}},
	{"$orthographic"s,    NodeMeta{NodeType::Orthographic,    compileOrthographic}},

	{"$render"s,          NodeMeta{NodeType::Render,          compileRender}},
	{"$renderToTexture"s, NodeMeta{NodeType::RenderToTexture, compileRenderToTexture}},

    {"$image"s,           NodeMeta{NodeType::Image,           compileImage}},
    {"$material"s,        NodeMeta{NodeType::Material,        compileMaterial}},

	{"$fxAuraForLaura"s,  NodeMeta{NodeType::FxAuraForLaura,  compileFxAuraForLaura}},
	{"$fxCarpet"s,        NodeMeta{NodeType::FxCarpet,        compileFxCarpet}},
	{"$fxFoo"s,           NodeMeta{NodeType::FxFoo,           compileFxFoo}},
	{"$fxMC"s,            NodeMeta{NodeType::FxMC,            compileFxMC}},
	{"$fxXYQuad"s,        NodeMeta{NodeType::FxXYQuad,        compileFxXYQuad}},
	{"$group"s,           NodeMeta{NodeType::Group,           compileGroup}},
	{"$layer"s,           NodeMeta{NodeType::Layer,           compileLayer}},
	{"$layerChooser"s,    NodeMeta{NodeType::LayerChooser,    compileLayerChooser}},
    {"$repeat"s,          NodeMeta{NodeType::Repeat,          compileRepeat}},
    {"$translate"s,       NodeMeta{NodeType::Translate,       compileTranslate}},
};


optional<tuple<NodeMeta, JsonValue, string>> jvPartialDecodeNode(const JsonValue& data) {
	if (data.getTag() == JSON_OBJECT) {
		// it's an object
		if (auto node = data.toNode(); node) {
			// with at least one node
			if (auto search = nodeTable.find(node->key); search != nodeTable.end()) {
				// the first node has a key with a valid node type
				const auto nodeMeta = search->second;
				if (node->value.getTag() == JSON_OBJECT) {
					// with an object as its value
					string guid;
					if (auto jv = jv_find(node->value, "id", JSON_STRING)) {
						// that object includes an id key with string value
						guid = jv->toString(); }
					else {
						guid = fmt::sprintf("__auto%d__", idGen++); }
					return tuple{nodeMeta, node->value, guid}; }}}}
	return {};}


CompileResult deserializeNode(const JsonValue& data, const rglv::MeshStore& meshStore) {
	if (auto decoded = jvPartialDecodeNode(data)) {
		const auto[nodeMeta, obj, guid] = *decoded;
		return nodeMeta.compileFunc(guid, obj, meshStore);}
	return {}; }


tuple<bool, NodeList> compile(const JsonValue& jsonItems, rglv::MeshStore& meshstore) {
	using std::cout;
	NodeList nodes;

	// add the json nodes
	bool success = true;
	for (const auto& jsonItem : jsonItems) {
		auto instance = deserializeNode(jsonItem->value, meshstore);
		if (instance.has_value()) {
			auto [node, deps] =  *instance;
			nodes.push_back(node);
			std::copy(deps.begin(), deps.end(), std::back_inserter(nodes)); }
		else {
			cout << "error deserializing json item\n";
			success = false; }}

	return tuple{success, nodes}; }


bool link(NodeList& nodes) {
	using std::cout;
	unordered_map<string, int> byId;

	// index by id, check for duplicates
	for (int idx=0; idx<nodes.size(); idx++) {
		const auto nodeName = nodes[idx]->name;
		if (auto existing = byId.find(nodeName); existing != byId.end()) {
			cout << "error: node id \"" << nodeName << "\" not unique\n";
			return false; }
		else {
			byId[nodeName] = idx; }}

	// resolve links, replace with pointers
	for (auto& node : nodes) {
		for (auto& input : node->inputs) {
			const auto&[destAttr, depNodeRef] = input;
			// cout << "node(" << node->name << ") will get input \"" << destAttr << "\" from " << depNodeRef << endl;

			// deserialize reference
			string depId;
			string depSlot;
			auto parts = explode(depNodeRef, ':');
			if (parts.size() == 1) {
				depId = parts[0];  depSlot = "default"s; }
			else {
				depId = parts[0];  depSlot = parts[1]; }

			if (auto search = byId.find(depId); search != byId.end()) {
				auto depNode = nodes[search->second].get();
				node->connect(destAttr, depNode, depSlot); } // XXX change order of args
			else {
				cout << "error: node for ref " << depNodeRef << " not found\n";
				return false; }}}

	return true; }

#undef Required
#undef Optional

}  // close package namespace
}  // close enterprise namespace
