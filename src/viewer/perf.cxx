#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_aligned_containers.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
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

using namespace rqdq;
using namespace PixelToaster;
using namespace rclma;
using namespace rclmt;
using namespace rclx;
using namespace rglr;
using namespace rglv;
using namespace rmls;
using namespace rmlv;
using namespace rclt;
using namespace rqv;
namespace jobsys = rqdq::rclmt::jobsys;
namespace framepool = rqdq::rclma::framepool;

constexpr int MEASUREMENT_SAMPLESIZE_IN_FRAMES = 1100;
constexpr double MEASUREMENT_DISCARD = 0.05;  // discard worst 5% because of os noise


struct AppConfig {
	ivec2 tileDim{ 8, 8 };         // blocks
	ivec2 canvasDim{ 1280, 720 };  // px
	float t{0.0F};                 // seconds
	int n{1000};                   // frames
	int priming{100};               // frames
	unsigned int concurrency{std::thread::hardware_concurrency()};  // threads

	std::vector<std::string> scenes{ {"data/scene.json"} }; };

#pragma pack(1)
struct TGAHead {
	uint8_t idLengh{0};  // no id
	uint8_t colormapTypeCode{0};  // no color map
	uint8_t dataTypeCode{2};  // uncompressed RGB
	int16_t colormapOrigin{0};  // no color map
	int16_t colormapLength{0};  // no color map
	uint8_t colormapDepth{0};  // no color map
	int16_t xOrigin{0};
	int16_t yOrigin{0};
	uint16_t width{0};
	uint16_t height{0};
	uint8_t depth{32};  // ARGB (b,g,r,a on disk)
	uint8_t imageDescriptor{0};

	TGAHead(uint16_t w, uint16_t h, uint8_t d=24) : width(w), height(h), depth(d) {} };
#pragma pack()


void Save(TrueColorCanvas& canvas, const std::string& fn) {
	const TGAHead head( uint16_t(canvas.width()), uint16_t(canvas.height()), 24 );
	ofstream out(fn, std::ios::binary);
	out.write((char*)&head, sizeof(head));
	std::vector<uint8_t> buf(canvas.width()*3);
	for (int y = canvas.height()-1; y >= 0; --y) {
		auto src = canvas.data() + y*canvas.stride();
		for (int i = 0; i < canvas.width(); ++i) {
			buf[i*3+0] = src[i].b;
			buf[i*3+1] = src[i].g;
			buf[i*3+2] = src[i].r; }
		out.write((const char*)buf.data(), canvas.width()*3); }}


auto ParseArgs(const int argc, const char* const * const argv) -> AppConfig {
	AppConfig config;
	for (int i=1; i<argc; ++i) {
		std::string arg{argv[i]};
		if (ConsumePrefix(arg, "-w")) {
			config.canvasDim.x = stoi(arg); }
		else if (ConsumePrefix(arg, "-h")) {
			config.canvasDim.y = stoi(arg); }
		else if (ConsumePrefix(arg, "-T")) {
			config.t = stof(arg); }
		else if (ConsumePrefix(arg, "-s")) {
			config.scenes[0] = arg; }
		else if (ConsumePrefix(arg, "-n")) {
			config.n = stoi(arg); }
		else if (ConsumePrefix(arg, "-tw")) {
			config.tileDim.x = stoi(arg); }
		else if (ConsumePrefix(arg, "-th")) {
			config.tileDim.y = stoi(arg); }
		else if (ConsumePrefix(arg, "-c")) {
			config.concurrency = std::clamp(stoi(arg), 1, 64); }
		else {
			std::cerr << "ignoring unknown arg \"" << arg << "\"\n"; }}
	return config; }


auto operator<<(std::ostream& s, const AppConfig& data) -> std::ostream& {
	s << "concurrency: " << data.concurrency << " threads\n";
	s << "sample size: " << data.n << " frames\n";
	s << "     canvas: " << data.canvasDim.x << "x" << data.canvasDim.y << " px\n";
	s << "       tile: " << data.tileDim.x << "x" << data.tileDim.y << " blocks\n";
	s << "      scene: " << data.scenes[0] << "\n";
	s << "          t: " << data.t << "\n";
	return s; }


auto operator<<(std::ostream& s, const BenchStat& data) -> std::ostream& {
	s << "   min    25th     med    75th     max    mean    sdev\n";
	fmt::memory_buffer out;
	format_to(out, "{: 6.2f}  ", data.min);
	format_to(out, "{: 6.2f}  ", data.p25);
	format_to(out, "{: 6.2f}  ", data.med);
	format_to(out, "{: 6.2f}  ", data.p75);
	format_to(out, "{: 6.2f}  ", data.max);
	format_to(out, "{: 6.2f}  ", data.avg);
	format_to(out, "{: 6.2f}  ", data.std);
	s << to_string(out) << "\n";
	return s; }


class Application {
	const AppConfig config_;

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
	Application(AppConfig config) :
		config_(config) {
		jobsys::telemetryEnabled = false;
		jobsys::init(config_.concurrency);
		// XXX depends on jobsys
		framepool::Init(); }

	~Application() {
		jobsys::stop();
		jobsys::join(); }

	int Main() {
		std::string textureDir{"data/texture"};
		//if (auto jv = jv_find(config_json.GetRoot(), "textureDir")) {
		//	textureDir = jv->toString(); }

		std::string meshDir{"data/mesh"};
		//if (auto jv = jv_find(config_json.GetRoot(), "meshDir")) {
		//	meshDir = jv->toString(); }

		textureStore_.load_dir(textureDir);
		meshStore_.LoadDir(meshDir);

		PrepareBuiltInNodes();

		// initial read of scene data
		for (const auto& sp : config_.scenes) {
			auto item = std::pair<JSONFile, NodeList>( JSONFile(sp), NodeList{} );
			userNodes_.emplace_back(item);
			auto& sj = userNodes_.back().first;
			if (!sj.IsValid()) {
				std::cerr << "error while reading \"" << sp << "\", can't continue.\n"; }}
		if (auto success = MaybeRecompile(/*force=*/true); !success) {
			std::cerr << "compile failed, can't continue.\n";
			return 1; }

		const int canvasAreaInPx = Area(config_.canvasDim);
		rcls::vector<PixelToaster::TrueColorPixel> outbuf(canvasAreaInPx);
		TrueColorCanvas canvas(outbuf.data(), config_.canvasDim.x, config_.canvasDim.y);

		// prepare globals
		globalsNode_->Upsert("wallclock", config_.t);
		globalsNode_->Upsert("windowSize", rmlv::vec2(float(config_.canvasDim.x), float(config_.canvasDim.y)));
		globalsNode_->Upsert("tileSize", rmlv::vec2(float(config_.tileDim.x), float(config_.tileDim.y)));
		globalsNode_->Upsert("windowAspect", config_.canvasDim.x / float(config_.canvasDim.y));

		const int N = config_.n;

		std::vector<double> measurementSamples_;
		measurementSamples_.reserve(N);
		PixelToaster::Timer timer;

		jobsys::work_start();

		for (int fn=0; fn<config_.priming; ++fn) {
			jobsys::reset();
			framepool::Reset();
			ComputeAndRenderFrame(canvas); }

		PixelToaster::Timer outerTimer;
		for (int fn=0; fn<N; ++fn) {
			timer.reset();
			jobsys::reset();
			framepool::Reset();
			ComputeAndRenderFrame(canvas);
			auto elapsed = timer.time() * 1000.0;
			measurementSamples_.push_back(elapsed); }

		auto outerElapsed = outerTimer.time();
		jobsys::work_end();

		std::cout << CalcStat(measurementSamples_, MEASUREMENT_DISCARD);

		{fmt::memory_buffer out;
		format_to(out, "total time: {:6.4f}s, per frame: {:6.4f}ms", outerElapsed, outerElapsed/N*1000.0F);
		std::cerr << to_string(out) << "\n";}

		std::cerr << "saving test.tga ... " << std::flush;
		Save(canvas, "test.tga");
		std::cerr << "OK\n";
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

		if (allUpToDate && !force) {
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
	auto config = ParseArgs(argc, argv);
	std::cout << config;


	int rtn{1};
	try {
		rtn = Application(config).Main(); }
	catch (const std::exception& err) {
		std::cout << "exception: " << err.what() << std::endl; }
	catch (...) {
		std::cout << "caught unknown exception\n"; }
	return rtn; }
