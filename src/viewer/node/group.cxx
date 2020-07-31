#include <atomic>
#include <memory>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/i_gl.hxx"

namespace rqdq {
namespace {

using namespace rqv;
namespace jobsys = rclmt::jobsys;

class Impl : public IGl {
public:
	Impl(std::string_view id, InputList inputs)
		:IGl(id, std::move(inputs)) {}

	bool Connect(std::string_view attr, NodeBase* other, std::string_view slot) override {
		if (attr == "gl") {
			IGl* tmp = dynamic_cast<IGl*>(other);
			if (tmp == nullptr) {
				TYPE_ERROR(IGl);
				return false; }
			gls_.push_back(tmp);
			return true; }
		return IGl::Connect(attr, other, slot); }

	void Main() override {
		jobsys::Job *doneJob = jobsys::make_job(jobsys::noop);
		AddLinksTo(doneJob);

		if (gls_.empty()) {
			jobsys::run(doneJob); }
		else {
			for (auto glnode : gls_) {
				glnode->AddLink(AfterAll(doneJob)); }
			for (auto glnode : gls_) {
				glnode->Run(); } }}

	static void AllThen(rclmt::jobsys::Job*, unsigned threadId [[maybe_unused]], std::tuple<std::atomic<int>*, rclmt::jobsys::Job*>* data) {
		auto [cnt, link] = *data;
		auto& counter = *cnt;
		if (--counter != 0) {
			return; }
		jobsys::run(link); }

	void Draw(int pass, rglv::GL* dc, const rmlm::mat4* pmat, const rmlm::mat4* mvmat, int depth) override {
		for (auto glnode : gls_) {
			// XXX increment depth even if matrix/transform stack is not modified?
			glnode->Draw(pass, dc, pmat, mvmat, depth); }}

protected:
	void AddDeps() override {
		for (auto item : gls_) {
			AddDep(item); }}

protected:
	std::vector<IGl*> gls_; };


class Compiler final : public NodeCompiler {
	void Build() override {
		if (auto jv = rclx::jv_find(data_, "gl", JSON_ARRAY)) {
			for (const auto& item : *jv) {
				if (item->value.getTag() == JSON_STRING) {
					inputs_.emplace_back("gl", item->value.toString()); } } }
		out_ = std::make_shared<Impl>(id_, std::move(inputs_)); }};


struct init { init() {
	NodeRegistry::GetInstance().Register("$group", [](){ return std::make_unique<Compiler>(); });
}} init{};


}  // namespace
}  // namespace rqdq
