#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
// #include "src/rgl/rglr/rglr_pixeltoaster_util.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/rml/rmls/rmls_bench.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/node/i_controller.hxx"
#include "src/viewer/node/i_output.hxx"
#include "src/viewer/node/multivalue.hxx"
#include "src/viewer/node/uicamera.hxx"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include <fmt/format.h>
#include <fmt/printf.h>
#include "3rdparty/gason/gason.h"
#include "3rdparty/pixeltoaster/PixelToaster.h"
// #include <Windows.h>

using namespace rqdq;
using namespace PixelToaster;
using namespace rclma;
using namespace rclmt;
using namespace rclx;
using namespace rglr;
using namespace rglv;
using namespace rmls;
using namespace rmlv;
using namespace rqv;
namespace jobsys = rqdq::rclmt::jobsys;
namespace framepool = rqdq::rclma::framepool;

constexpr int MEASUREMENT_SAMPLESIZE_IN_FRAMES = 1100;
constexpr double MEASUREMENT_DISCARD = 0.05;  // discard worst 5% because of os noise

class Application {

	rglv::MeshStore meshStore_;
	TextureStore textureStore_;

	HandyCam camera_;

	// current scene, must always be valid & linked
	NodeList nodes_;

	// user node files
	// nodes match last-known-good compile for its corresponding file
	std::vector<std::pair<JSONFile, NodeList>> userNodes_;

	// built-in nodes
	NodeList appNodes_;
	std::shared_ptr<MultiValueNode> globalsNode_;
	std::shared_ptr<MultiValueNode> syncNode_;
	std::shared_ptr<UICamera> uiCameraNode_;

public:
	int Main(int argc, char**argv) {
		JSONFile config_json("data/viewer_config.json");

		std::vector<std::string> scenePath = { { "data/scene.json" } };
		if (auto jv = jv_find(config_json.GetRoot(), "scene", JSON_STRING)) {
			scenePath[0] = jv->toString(); }
		else if (auto ja = jv_find(config_json.GetRoot(), "scene", JSON_ARRAY)) {
			scenePath.clear();
			for (const auto item : *ja) {
				scenePath.emplace_back(item->value.toString()); } }

		std::string textureDir{"data/texture"};
		if (auto jv = jv_find(config_json.GetRoot(), "textureDir")) {
			textureDir = jv->toString(); }

		std::string meshDir{"data/mesh"};
		if (auto jv = jv_find(config_json.GetRoot(), "meshDir")) {
			meshDir = jv->toString(); }

		textureStore_.load_dir(textureDir);
		meshStore_.LoadDir(meshDir);

		PrepareBuiltInNodes();

		// initial read of scene data
		for (const auto& sp : scenePath) {
			auto item = std::pair<JSONFile, NodeList>( JSONFile(sp), NodeList{} );
			userNodes_.emplace_back(item);
			auto& sj = userNodes_.back().first;
			if (!sj.IsValid()) {
				std::cerr << "error while reading \"" << sp << "\", can't continue.\n"; }}
		if (auto success = MaybeRecompile(/*force=*/true); !success) {
			std::cerr << "compile failed, can't continue.\n";
			return 1; }

		const ivec2 tile_dim{ 8, 8 };
		constexpr int canvasWidthInPx = 1280;
		constexpr int canvasHeightInPx = 720;
		constexpr int canvasAreaInPx = canvasWidthInPx * canvasHeightInPx;
		rcls::vector<PixelToaster::TrueColorPixel> outbuf(canvasAreaInPx);
		TrueColorCanvas canvas(outbuf.data(), canvasWidthInPx, canvasHeightInPx);

		// update globals
		const auto tNow = 0.0F;
		globalsNode_->Upsert("wallclock", tNow);
		globalsNode_->Upsert("windowSize", rmlv::vec2(float(canvasWidthInPx), float(canvasHeightInPx)));
		globalsNode_->Upsert("tileSize", rmlv::vec2(float(tile_dim.x), float(tile_dim.y)));
		globalsNode_->Upsert("windowAspect", float(canvasWidthInPx) / float(canvasHeightInPx));

		constexpr int primeFrames = 100;
		constexpr int measureFrames = 1000;

		std::vector<double> measurementSamples_;
		measurementSamples_.reserve(measureFrames);

		PixelToaster::Timer outerTimer;
		PixelToaster::Timer timer;

		jobsys::work_start();
		for (int fn=0; fn<primeFrames+measureFrames; ++fn) {
			timer.reset();
			jobsys::reset();
			framepool::Reset();

			if (0) {
				const std::string_view selector{"game"};
				const auto match = rclr::find_if(nodes_, [=](const auto& node) { return node->get_id() == selector; });
				if (match == end(nodes_)) {
					/*std::cerr << "BF controller node \"" << selector << "\" not found\n";*/ }
				else {
					auto* node = dynamic_cast<IController*>(match->get());
					if (node == nullptr) {
						std::cerr << "BF node with id\"" << selector << "\" is not an IController\n"; }
					else {
						node->BeforeFrame(tNow); }}}

			ComputeAndRenderFrame(canvas);

			auto elapsed = timer.time() * 1000.0;
			if (fn == primeFrames) {
				outerTimer.reset(); }
			if (fn > primeFrames) {
				measurementSamples_.push_back(elapsed); }}

		auto outerElapsed = outerTimer.time();
		jobsys::work_end();

		auto res = CalcStat(measurementSamples_, MEASUREMENT_DISCARD);
		std::cerr << "   min    25th     med    75th     max    mean    sdev\n";
		{fmt::memory_buffer out;
		format_to(out, "{: 6.2f}  ", res.min);
		format_to(out, "{: 6.2f}  ", res.p25);
		format_to(out, "{: 6.2f}  ", res.med);
		format_to(out, "{: 6.2f}  ", res.p75);
		format_to(out, "{: 6.2f}  ", res.max);
		format_to(out, "{: 6.2f}  ", res.avg);
		format_to(out, "{: 6.2f}  ", res.std);
		std::cerr << to_string(out) << "\n";}

		{fmt::memory_buffer out;
		format_to(out, "total time: {:6.4f}s, per frame: {:6.4f}ms", outerElapsed, outerElapsed/measureFrames*1000.0F);
		std::cerr << to_string(out) << "\n";}

		std::cerr << "done.\n";
		return 0; }

private:

	void PrepareBuiltInNodes() {
		globalsNode_ = make_shared<MultiValueNode>("globals", InputList());
		appNodes_.emplace_back(globalsNode_);

		syncNode_ = make_shared<MultiValueNode>("sync", InputList());
		appNodes_.emplace_back(syncNode_);

		uiCameraNode_ = make_shared<UICamera>("uiCamera", InputList(), camera_);
		appNodes_.emplace_back(uiCameraNode_); }

	void ComputeAndRenderFrame(TrueColorCanvas canvas) {
		const std::string_view selector{"root"};

		const auto match = rclr::find_if(nodes_, [=](const auto& node) { return node->get_id() == selector; });
		if (match == end(nodes_)) {
			std::cerr << "output node \"" << selector << "\" not found\n";
			return; }

		auto* node = dynamic_cast<IOutput*>(match->get());
		if (node == nullptr) {
			std::cerr << "node with id \"" << selector << "\" is not an OutputNode\n";
			return; }

		auto rootJob = jobsys::make_job(jobsys::noop);
		for (auto& n : nodes_) {
			n->Reset(); }
		ComputeIndegreesFrom(node);
		node->set_indegreeWaitCnt(1);
		node->SetOutputCanvas(&canvas);
		node->AddLink(rootJob);
		node->Run();
		jobsys::wait(rootJob); }

	auto MaybeRecompile(bool force=false) -> bool {
		bool allUpToDate{true};
		for (auto& un : userNodes_) {
			auto& sceneJson = un.first;
			if (sceneJson.IsOutOfDate()) {
				allUpToDate = false; }}

		if (allUpToDate and !force) {
			return false; }

		// debounce/settle time
		jobsys::_sleep(200);

		PixelToaster::Timer compileTime;
		for (auto& un : userNodes_) {
			auto& sceneJson = un.first;
			auto& nodeList = un.second;
			if (force || sceneJson.IsOutOfDate()) {
				sceneJson.Refresh();
				if (sceneJson.IsValid()) {
					NodeList newNodes;
					bool success;
					std::tie(success, newNodes) = CompileDocument(sceneJson.GetRoot(), meshStore_);
					if (!success) {
						std::cerr << "compile failed \"" << sceneJson.Path() << "\" nodes not updated\n";
						return false; }
					nodeList = std::move(newNodes); }}}

		NodeList newNodes;
		rclr::for_each(nodes_, [](auto& n){ n->DisconnectAll(); });
		newNodes.insert(end(newNodes), begin(appNodes_), end(appNodes_));
		for (auto& un : userNodes_) {
			auto& nodeList = un.second;
			newNodes.insert(end(newNodes), begin(nodeList), end(nodeList)); }
		if (Link(newNodes)) {
			// new node collection is good!
			auto elapsed = compileTime.delta() * 1000.0;
			nodes_ = std::move(newNodes);
			std::cerr << fmt::sprintf("scene compiled in %.2fms\n", elapsed);
			return true; }

		// new node-set failed, re-link the previous set
		if (!Link(nodes_)) {
			std::cerr << "re-link old nodes failed!\n";
			std::exit(1); }
		return false; }};


int main(int argc, char** argv) {
	JSONFile config_json("data/viewer_config.json");
	int threads = std::thread::hardware_concurrency();

	if (auto jv = jv_find(config_json.GetRoot(), "concurrency", JSON_NUMBER)) {
		const auto concurrency = int(jv->toNumber());
		if (concurrency <= 0) {
			threads = int(std::thread::hardware_concurrency()) + concurrency; }
		else {
			threads = concurrency; } }

	jobsys::telemetryEnabled = true;
	jobsys::init(threads); // threads);
	framepool::Init();

	int rtn = 111;
	try {
		rtn = Application().Main(argc, argv); }
	catch (const std::exception& err) {
		std::cout << "exception: " << err.what() << std::endl; }
	catch (...) {
		std::cout << "caught unknown exception\n"; }

	jobsys::stop();
	jobsys::join();
	return rtn; }
