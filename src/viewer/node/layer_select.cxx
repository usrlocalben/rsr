#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_layer.hxx"
#include "src/viewer/node/i_value.hxx"

namespace rqdq {
namespace {

using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl : public ILayer {
public:
	using ILayer::ILayer;

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "layer") {
			auto tmp = dynamic_cast<ILayer*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(ILayer);
				return false; }
			layers_.push_back(tmp);
			return true; }
		if (attr == "selector") {
			selectorNode_ = dynamic_cast<IValue*>(other);
			selectorSlot_ = slot;
			if (selectorNode_ == nullptr) {
				TYPE_ERROR(IValue);
				return false; }
			return true; }
		return ILayer::Connect(attr, other, slot); }

	void Main() override {
		using rmlv::ivec2;
		jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
		AddLinksTo(doneJob);

		auto layerNode = GetSelectedLayer();
		if (layerNode == nullptr) {
			jobsys::run(doneJob); }
		else {
			layerNode->AddLink(AfterAll(doneJob));
			layerNode->Run(); }}

	auto Color() -> rmlv::vec3 override {
		auto layerNode = GetSelectedLayer();
		if (layerNode == nullptr) {
			return rmlv::vec3{ 0.0F }; }
		return layerNode->Color(); }

	void Render(int pass, rglv::GL* dc, rmlv::ivec2 targetSizeInPx, float aspect) override {
		auto layerNode = GetSelectedLayer();
		if (layerNode != nullptr) {
			layerNode->Render(pass, dc, targetSizeInPx, aspect); }}

protected:
	void AddDeps() override {
		ILayer::AddDeps();
		AddDep(GetSelectedLayer()); }

private:	
	ILayer* GetSelectedLayer() const {
		if (layers_.empty()) {
			return nullptr; }
		auto idx = static_cast<int>(selectorNode_->Eval(selectorSlot_).as_float());
		const int lastLayer = int(layers_.size() - 1);
		if (idx < 0 || lastLayer < idx) {
			return nullptr; }
		return layers_[idx]; }

	std::vector<ILayer*> layers_;
	IValue* selectorNode_{nullptr};
	std::string selectorSlot_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (!Input("selector", /*required=*/true)) { return; }
		if (auto jv = rclx::jv_find(data_, "layers", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("layer", item->value.toString()); } } }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$layerSelect", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
