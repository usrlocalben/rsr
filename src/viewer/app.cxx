#include "app.hxx"

#ifdef ENABLE_MUSIC
#include "src/ral/ralio/ralio_audio_controller.hxx"
#include "src/ral/rals/rals_sync_controller.hxx"
#endif
#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_smoothedintervaltimer.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rgl/rglr/rglr_pixeltoaster_util.hxx"
#include "src/rgl/rglr/rglr_profont.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_gpu.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/rml/rmls/rmls_bench.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/jobsys_vis.hxx"
#include "src/viewer/node/multivalue.hxx"
#include "src/viewer/node/i_output.hxx"
#include "src/viewer/node/i_controller.hxx"
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
#include <Windows.h>

namespace rqdq {
namespace {

auto boolify(std::string s) {
	rclt::ToLower(s);
	s = rclt::Trim(s);
	return !(s=="0" || s=="no" || s=="false" || s=="off"); }


auto stoivec2(std::string s) -> rmlv::ivec2 {
	auto segs = rclt::Split(s, ',');
	return { stoi(segs[0]), stoi(segs[1]) }; }

}  // close unnamed namespace
namespace rqv {

using namespace PixelToaster;
#ifdef ENABLE_MUSIC
using namespace ralio;
#endif
using namespace rclma;
using namespace rclmt;
using namespace rclx;
using namespace rglr;
using namespace rglv;
using namespace rmls;
using namespace rmlv;
// using namespace std;

const int MEASUREMENT_SAMPLESIZE_IN_FRAMES = 1100;
const int SCAN_SAMPLESIZE_IN_FRAMES = 120;
const ivec2 SCAN_SIZE_LIMIT_IN_TILES{ 16, 16 };
const double MEASUREMENT_DISCARD = 0.05;  // discard worst 5% because of os noise

std::vector<ivec2> modelist = {
	{640, 360},
	{640, 480},
	{800, 600},
	{1280, 720},
	{1280, 800},
	{1360, 768},
	{1920, 1080},
	{1920, 1200},
	{320, 240}, };


struct Soundtrack {
	std::string path;
	double tempoInBeatsPerMinute;
	double durationInSeconds;};


struct SoundtrackSerializer {
	static
	auto Deserialize(const JsonValue data) -> std::optional<Soundtrack> {
		double tempo;
		double duration = 0;
		string path;

		if (auto search = jv_find(data, "tempo", JSON_NUMBER)) {
			tempo = search->toNumber(); }
		else {
			std::cerr << "soundtrack: missing tempo\n";
			return {}; }

		if (auto search = jv_find(data, "path", JSON_STRING)) {
			path = search->toString(); }
		else {
			std::cerr << "soundtrack: missing path\n";
			return {}; }

		if (auto dnode = jv_find(data, "duration", JSON_OBJECT)) {
			if (auto search = jv_find(*dnode, "minutes", JSON_NUMBER)) {
				duration += search->toNumber() * 60.0; }
			if (auto search = jv_find(*dnode, "seconds", JSON_NUMBER)) {
				duration += search->toNumber(); }
			if (duration == 0) {
				std::cerr << "soundtrack: duration node should indicate duration in minutes and/or seconds\n";
				return {}; }}
		else {
			std::cerr << "soundtrack: missing duration node\n";
			return {}; }

		return Soundtrack{ path, tempo, duration };}};


struct SyncConfig {
	int precisionInRowsPerBeat;
	vector<string> trackNames; };


struct SyncConfigSerializer {
	static
	auto Deserialize(const JsonValue data) -> std::optional<SyncConfig> {
		SyncConfig sc;

		if (auto search = jv_find(data, "precision", JSON_NUMBER)) {
			sc.precisionInRowsPerBeat = int(search->toNumber()); }
		else {
			std::cerr << "syncConfig: missing precision\n";
			return {}; }

		if (auto search = jv_find(data, "tracks", JSON_ARRAY)) {
			for (const auto item : *search) {
				sc.trackNames.emplace_back(item->value.toString()); }}
		if (sc.trackNames.empty()) {
			std::cerr << "syncConfig: warning: no track names configured\n"; }
		return sc; }};


AppConfig defaultConfig{
	/*configPath*/"data/viewer_config.json",
	/*debug=*/true,
	/*telemetryScale=*/30,
	/*nice=*/false,
	/*concurrency=*/0,
	/*nodePath=*/{ {"data/scene.json"} },
	/*latencyInFrames=*/1,
	/*textureDir=*/"data/texture",
	/*meshDir=*/"data/mesh",
	/*fullScreen=*/false,
	/*outputSizeInPx=*/rmlv::ivec2(1280, 720),
    /*tileSizeInBlocks=*/rmlv::ivec2(8, 8) };


class Application::impl : public PixelToaster::Listener {

	const AppConfig config_;
	const bool nice_;
	const std::vector<std::string>& nodePath_;

	// mutable config
	ivec2 tileSizeInBlocks_;
	bool wantDebug_;
	int telemetryScale_;
	bool showTileThreads_{false};
	bool shouldQuit_{false};
	bool isPaused_{false};
	bool wantFullScreen_;
	ivec2 wantedSizeInPx_;
	bool wantModeList_{false};

	rglv::MeshStore meshStore_;
	TextureStore textureStore_;
	PixelToaster::Display display_;
	rcls::SmoothedIntervalTimer refreshTime_;
	PixelToaster::Timer renderTime_;
	double lastRenderTime_;
	PixelToaster::Timer wallClock_;

	// BEGIN debugger state
	int runtimeInFrames_{ 0 };

	bool mouseCaptured_ = false;
	bool resetMouseNextFrame_ = false;
	rmlv::vec2 mousePositionInPx_{};

	bool measuring_ = false;
	bool startMeasuring_ = false;
	bool scanning_ = false;
	bool startScanning_ = false;
	bool stopScanning_ = false;
	double scanMinValue_;
	ivec2 scanMinDim_;

	std::vector<double> measurementSamples_;
	std::optional<BenchStat> lastStats_;
	ivec2 windowSizeInPx_{ 0, 0 };
	bool runningFullScreen_ = false;
	bool keys_shifted = false;

	HandyCam camera_;
	// END debugger state

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

#ifdef ENABLE_MUSIC
	// music/sync
	Soundtrack soundtrack_;
	SyncConfig syncConfig_;
#endif

public:
	impl(AppConfig config) :
		config_(Merge(defaultConfig, config)),
		nice_(config_.nice.value()),
		nodePath_(config_.nodePath.value()),
		tileSizeInBlocks_(config_.tileSizeInBlocks.value()),
		wantDebug_(config_.debug.value()),
		telemetryScale_(config_.telemetryScale.value()),
		wantFullScreen_(config_.fullScreen.value()),
		wantedSizeInPx_(config_.outputSizeInPx.value()) {}
	
	void Run() {
		textureStore_.load_dir(config_.textureDir.value());
		meshStore_.LoadDir(config_.meshDir.value());

		PrepareBuiltInNodes();

		// initial read of scene data
		for (const auto& sp : nodePath_) {
			auto item = std::pair<JSONFile, NodeList>( JSONFile(sp), NodeList{} );
			userNodes_.emplace_back(item);
			auto& sj = userNodes_.back().first;
			if (!sj.IsValid()) {
				std::cerr << "error while reading \"" << sp << "\", can't continue.\n"; }}
		if (auto success = MaybeRecompile(/*force=*/true); !success) {
			std::cerr << "compile failed, can't continue.\n";
			return; }


#ifdef ENABLE_MUSIC
		JSONFile config_json(config_.configPath.value());
		if (auto jv = jv_find(config_json.GetRoot(), "soundtrack", JSON_OBJECT)) {
			if (auto result = SoundtrackSerializer::Deserialize(*jv)) {
				soundtrack_ = result.value(); }
			else {
				return; }}
		else {
			std::cerr << "compiled with ENABLE_MUSIC but soundtrack config not found, can't continue\n";
			return; }

		if (auto jv = jv_find(config_json.GetRoot(), "sync", JSON_OBJECT)) {
			if (auto result = SyncConfigSerializer::Deserialize(*jv)) {
				syncConfig_ = result.value(); }
			else {
				return; }}
		else {
			std::cerr << "compiled with ENABLE_MUSIC but sync config not found, can't continue\n";
			return; }

		AudioController audioController;

		auto result = audioController.CreateStream(soundtrack_.path);
		if (!result.has_value()) {
			std::cerr << "failed to load " << soundtrack_.path << ", can't continue.\n";
			return; }

		auto soundtrack = std::move(result.value());

		const auto rowsPerSecond = soundtrack_.tempoInBeatsPerMinute / 60.0 * syncConfig_.precisionInRowsPerBeat;
		const auto songDurationInRows = int(soundtrack_.durationInSeconds * rowsPerSecond);

		rals::SyncController syncController(std::string("data/sync"), soundtrack, rowsPerSecond);
#ifndef SYNC_PLAYER
		syncController.Connect();
#endif
		for (const auto& name : syncConfig_.trackNames) {
			syncController.AddTrack(name); }

		audioController.Start();
		soundtrack.Play();
#endif //ENABLE_MUSIC

		if (!nice_) { jobsys::work_start();}

		try {
			while (!shouldQuit_) {
				MaybeUpdateDisplay();

				MaybeRecompile();

				if (mouseCaptured_ && resetMouseNextFrame_) {
					resetMouseNextFrame_ = false;
					display_.center_mouse(); }

				renderTime_.reset();
				auto frame = Frame(display_);
				auto canvas = frame.canvas();

				if (nice_) {
					jobsys::work_start(); }
				jobsys::reset();
				framepool::Reset();

#ifdef ENABLE_MUSIC
				double musicPositionInRows = syncController.GetPositionInRows();
				syncController.ForEachValue(musicPositionInRows, [this](const auto& name, auto value) {
					syncNode_->Upsert(name, float(value)); });

#ifdef SYNC_PLAYER
				// terminate when song is over
				if (musicPositionInRows > songDurationInRows) {
					shouldQuit_ = true; }  // end of soundtrack == end of demo
#else
				(void)songDurationInRows;
#endif

#ifdef SYNC_PLAYER
#else
				// host send-and-receive
				if (syncController.Update((int)floor(musicPositionInRows))) {
					syncController.Connect(); }
#endif
#endif // ENABLE_MUSIC

				// update globals
				const auto tNow = static_cast<float>(wallClock_.time());
				globalsNode_->Upsert("wallclock", isPaused_ ? float(0) : tNow);
				globalsNode_->Upsert("windowSize", rmlv::vec2(float(windowSizeInPx_.x), float(windowSizeInPx_.y)));
				globalsNode_->Upsert("tileSize", rmlv::vec2(float(tileSizeInBlocks_.x), float(tileSizeInBlocks_.y)));
				globalsNode_->Upsert("windowAspect", windowSizeInPx_.x / float(windowSizeInPx_.y));

				{
					const std::string_view selector{"game"};
					const auto match = rclr::find_if(nodes_, [=](const auto& node) { return node->get_id() == selector; });
					if (match == end(nodes_)) {
						/*std::cerr << "BF controller node \"" << selector << "\" not found\n";*/ }
					else {
						auto* node = dynamic_cast<IController*>(match->get());
						if (node == nullptr) {
							std::cerr << "BF node with id\"" << selector << "\" is not an IController\n"; }
						else {
							node->BeforeFrame(isPaused_ ? float(0) : tNow); }}}

				ComputeAndRenderFrame(canvas);
				if (nice_) { jobsys::work_end();}

#ifdef ENABLE_MUSIC
				audioController.FillBuffers();  // decrease chance of missing vsync
#endif

				lastRenderTime_ = renderTime_.time();
				refreshTime_.Sample();
				DrawUI(canvas);
				runtimeInFrames_++; }}

		catch (WindowClosed) {
			std::cerr << "window was closed" << endl; }
		catch (WrongFormat) {
			std::cerr << "framebuffer not rgba8888" << endl; }

		if (!nice_) { jobsys::work_end();}

		std::cerr << "terminated.\n";
#ifdef ENABLE_MUSIC
#ifndef SYNC_PLAYER
		syncController.SaveTracks();
#endif
#endif
		}

private:
	auto defaultKeyHandlers() const -> bool override {
		return false; }

	void onKeyPressed(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		if (!wantDebug_) {
			if (key == Key::F1) {
				wantDebug_ = !wantDebug_;
				return; }

			const std::string_view selector{"game"};
			const auto match = rclr::find_if(nodes_, [=](const auto& node) { return node->get_id() == selector; });
			if (match == end(nodes_)) {
				std::cerr << "controller node \"" << selector << "\" not found\n";
				return; }
			auto* node = dynamic_cast<IController*>(match->get());
			if (node == nullptr) {
				std::cerr << "node with id\"" << selector << "\" is not an IController\n";
				return; }

			node->KeyPress(key);
			return; }

		switch (key) {
		case Key::OpenBracket: {
			int n = std::clamp(tileSizeInBlocks_.x - 1, 4, 16);
			tileSizeInBlocks_ = { n, n }; }
			break;
		case Key::CloseBracket: {
			int n = std::clamp(tileSizeInBlocks_.x + 1, 4, 16);
			tileSizeInBlocks_ = { n, n }; }
			break;
		case Key::F1:
			wantDebug_ = !wantDebug_;
			break;
		case Key::F2:
			showTileThreads_ = !showTileThreads_;
			break;
		case Key::P:
			std::cout << camera_ << "\n";
			isPaused_= !isPaused_;
			break;
		case Key::W: camera_.MoveForward(); break;
		case Key::S: camera_.MoveBackward(); break;
		case Key::A: camera_.MoveLeft(); break;
		case Key::D: camera_.MoveRight(); break;
		case Key::E: camera_.MoveUp();  break;
		case Key::Q: camera_.MoveDown();  break;
		case Key::Period:
			telemetryScale_ += max(telemetryScale_ / 10, 1);
			break;
		case Key::Comma:
			telemetryScale_ = max(1, telemetryScale_ - (telemetryScale_ / 10));
			break;
		case Key::R:
			startMeasuring_ = true;
			break;
		case Key::C:
			lastStats_.reset();
			break;
		case Key::F:
			wantFullScreen_ = !wantFullScreen_;
			break;
		case Key::G:
			rglv::doubleBuffer = !rglv::doubleBuffer;
			break;
		case Key::N:
			mouseCaptured_ = !mouseCaptured_;
			if (mouseCaptured_) {
				display.capture_mouse(); }
			else {
				display.uncapture_mouse(); }
			break;
	/*
		case Key::S:
			if (!measuring_ && !scanning_) {
				startScanning_ = true; }
			else if (scanning_) {
				stopScanning_ = true; }
			break;
	*/
		default:
	//		cout << "key was " << key << endl;
			if (wantModeList_ && (key >= Key::One && key < (Key::One + modelist.size()))) {
				int modeidx = key - Key::One;
				wantedSizeInPx_ = modelist[modeidx]; }}}

	void onKeyDown(PixelToaster::DisplayInterface&, PixelToaster::Key key) override {
		switch (key) {
		case Key::Escape:
			shouldQuit_ = true;
			break;
		case Key::M:
			wantModeList_ = true;
			break;
		case Key::Shift:
			keys_shifted = true;
			break;
		default: break; }}

	void onKeyUp(PixelToaster::DisplayInterface&, PixelToaster::Key key) override {
		switch (key) {
		case Key::M:
			wantModeList_ = false;
			break;
		case Key::Shift:
			keys_shifted = false;
			break;
		default: break; }}

//	void onMouseButtonDown(PixelToaster::DisplayInterface & display, PixelToaster::Mouse mouse);
	void onMouseMove(PixelToaster::DisplayInterface& display, PixelToaster::Mouse mouse) override {
		mousePositionInPx_ = { mouse.x, mouse.y };
		float dmx = (display.width() / 2.0F) - mouse.x;
		float dmy = (display.height() / 2.0F) - mouse.y;
		if (mouseCaptured_) {
			//cout << "mm(" << dmx << ", " << dmy << ")" << endl;
			camera_.OnMouseMove({ dmx, dmy });
			resetMouseNextFrame_ = true; }}

	void onMouseWheel(PixelToaster::DisplayInterface&, PixelToaster::Mouse, short wheel_amount) override {
		int ticks = wheel_amount / 120;
		camera_.OnMouseWheel(ticks); }

	void DrawUI(TrueColorCanvas& canvas) {
		auto renderTimeInMillis = lastRenderTime_ * 1000.0; // renderTime_.Get();
		auto refreshTimeInMillis = refreshTime_.Get();

		auto tty = ProPrinter(canvas);

		if (wantModeList_) {
			int idx = 1;
			int top = canvas.height() / 2;
			for (const auto& mode : modelist) {
				fmt::memory_buffer buf;
				format_to(buf, "{}: {}x{}", idx, mode.x, mode.y);
				if (windowSizeInPx_ == mode) {
					format_to(buf, " (current)"); }
				tty.Write({ 16, top }, buf);
				top += 10;
				idx += 1; } }

		if (startMeasuring_) {
			measuring_ = true;
			startMeasuring_ = false;
			lastStats_.reset();
			measurementSamples_.clear(); }

		if (measuring_) {
			if (measurementSamples_.size() == MEASUREMENT_SAMPLESIZE_IN_FRAMES) {
				measuring_ = false;
				lastStats_ = CalcStat(measurementSamples_, MEASUREMENT_DISCARD); }
			else {
				measurementSamples_.push_back(renderTimeInMillis);
				fmt::memory_buffer buf;
				format_to(buf, "measuring, {} / {}", measurementSamples_.size(), MEASUREMENT_SAMPLESIZE_IN_FRAMES);
				tty.Write({ 16, 100 }, buf); } }

		if (startScanning_) {
			scanning_ = true;
			startScanning_ = false;
			tileSizeInBlocks_ = ivec2{ 2, 2 };
			scanMinDim_ = ivec2{ 2, 2 };
			scanMinValue_ = 10000.0;
			measurementSamples_.clear(); }

		if (stopScanning_) {
			stopScanning_ = false;
			scanning_ = false;
			tileSizeInBlocks_ = scanMinDim_; }

		if (scanning_) {
			{
				int top = 100;
				fmt::memory_buffer buf;
				format_to(buf, "probing for fastest file dimensions: {} / {}   ",
				          ((tileSizeInBlocks_.y - 1) * 16) + tileSizeInBlocks_.x-1, 16*16);
				tty.Write({ 16, top }, buf);  top += 10;  buf.clear();

				format_to(buf, "                     fastest so far: {}x{}   ",
				          scanMinDim_.x, scanMinDim_.y);
				tty.Write({ 16, top }, buf);  top += 10;  buf.clear();

				format_to(buf, "          press s to stop            ");
				tty.Write({ 16, top }, buf);  top += 10; }

			if (measurementSamples_.size() == SCAN_SAMPLESIZE_IN_FRAMES) {
				lastStats_ = CalcStat(std::vector<double>(begin(measurementSamples_) + 60, end(measurementSamples_)), MEASUREMENT_DISCARD);
				measurementSamples_.clear();
				if (lastStats_->avg < scanMinValue_) {
					scanMinValue_ = lastStats_->avg;
					scanMinDim_ = tileSizeInBlocks_; }
				tileSizeInBlocks_.x += 2;
				if (tileSizeInBlocks_.x > SCAN_SIZE_LIMIT_IN_TILES.x) {
					tileSizeInBlocks_.x = 2;
					tileSizeInBlocks_.y += 2; }
				if (tileSizeInBlocks_.y > SCAN_SIZE_LIMIT_IN_TILES.y) {
					scanning_ = false;
					tileSizeInBlocks_ = scanMinDim_; } }
			else {
				measurementSamples_.push_back(renderTimeInMillis); } }

		if (wantDebug_) {
			double fps = 1.0 / (refreshTimeInMillis / 1000.0);
			fmt::memory_buffer buf;
			format_to(buf, "{: 6.2f} ms, fps: {:.0f}", renderTimeInMillis, fps);
			tty.Write({ 16, 16 }, buf); }

		if (wantDebug_) {
			fmt::memory_buffer buf;
			format_to(buf, "tile size: {}x{}", tileSizeInBlocks_.x, tileSizeInBlocks_.y);
			format_to(buf, ", visu scale: {}", telemetryScale_);
			if (isPaused_) {
				format_to(buf, "   PAUSED"); }
			tty.Write({ 16, 27 }, buf); }

		if (wantDebug_) {
			tty.Write({ 16, -32 }, "F1 debug         l  toggle srgb   [&] tile size    ,&. vis scale       ");
			tty.Write({ 16, -42 }, " p toggle pause  m  change mode    r  measure       n  wasd capture    ");
			tty.Write({ 16, -52 }, " f fullscreen   F2  show tiles     g  dblbuf        shift -&+ grid size"); }
		else if (runtimeInFrames_ < (5 * 60)) {
			int top = canvas.height() - 11;
			tty.Write({ 0, top }, "F1 debug"); }

		if (wantDebug_) {
			render_jobsys(20, 40, float(telemetryScale_), canvas); }

		if (wantDebug_ && lastStats_) {
			int top = canvas.height() / 2;
			tty.Write({ 32, top }, "   min    25th     med    75th     max    mean    sdev");
			top += 10;
			fmt::memory_buffer out;
			format_to(out, "{: 6.2f}  ", lastStats_->min);
			format_to(out, "{: 6.2f}  ", lastStats_->p25);
			format_to(out, "{: 6.2f}  ", lastStats_->med);
			format_to(out, "{: 6.2f}  ", lastStats_->p75);
			format_to(out, "{: 6.2f}  ", lastStats_->max);
			format_to(out, "{: 6.2f}  ", lastStats_->avg);
			format_to(out, "{: 6.2f}  ", lastStats_->std);
			tty.Write({ 32, top }, out);
			top += 10;
			tty.Write({ 32, top }, "press C to clear"); }}

	void PrepareBuiltInNodes() {
		globalsNode_ = make_shared<MultiValueNode>("globals", InputList());
		appNodes_.emplace_back(globalsNode_);

		syncNode_ = make_shared<MultiValueNode>("sync", InputList());
		appNodes_.emplace_back(syncNode_);

		uiCameraNode_ = make_shared<UICamera>("uiCamera", InputList(), camera_);
		appNodes_.emplace_back(uiCameraNode_); }

	void MaybeUpdateDisplay() {
		if (windowSizeInPx_ != wantedSizeInPx_ || runningFullScreen_ != wantFullScreen_) {
			windowSizeInPx_ = wantedSizeInPx_;
			runningFullScreen_ = wantFullScreen_;
			preferTripleBuffering(config_.latencyInFrames == 2);
			display_.close();
			display_.open("rqdq 2021",
						  windowSizeInPx_.x, windowSizeInPx_.y,
						  runningFullScreen_ ? Output::Fullscreen : Output::Windowed,
						  Mode::TrueColor);
			display_.listener(this);
			display_.zoom(-1.0F); }}  // auto-scale based on windows' scaling factor

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
			fmt::print(stderr, "scene compiled in {:.2f}ms\n", elapsed);
			return true; }

		// new node-set failed, re-link the previous set
		if (!Link(nodes_)) {
			fmt::print(stderr, "re-link old nodes failed!\n");
			std::exit(1); }
		return false; }};


Application::Application(AppConfig c) :impl_(std::make_unique<impl>(c)) {}
Application::~Application() = default;
Application& Application::operator=(Application&&) noexcept = default;
Application& Application::Run() {
	impl_->Run();
	return *this;}




auto GetEnvConfig() -> AppConfig {
	using std::getenv;
	AppConfig out;

	char *s;
	if (s = getenv("RQDQ__VIEWER__CONFIG_PATH"); s) {
		out.configPath = s; }
	if (s = getenv("RQDQ__VIEWER__DEBUG"); s) {
		out.debug = boolify(s); }
	if (s = getenv("RQDQ__VIEWER__TELEMETRY_SACLE"); s) {
		out.telemetryScale = atoi(s); }
	if (s = getenv("RQDQ__VIEWER__NICE"); s) {
		out.nice = boolify(s); }
	if (s = getenv("RQDQ__VIEWER__CONCURRENCY"); s) {
		out.concurrency = atoi(s); }
	if (s = getenv("RQDQ__VIEWER__NODE_SOURCE"); s) {
		out.nodePath = rclt::Split(s, ','); }
	if (s = getenv("RQDQ__VIEWER__LATENCY"); s) {
		out.latencyInFrames = atoi(s); }
	if (s = getenv("RQDQ__VIEWER__TEXTURE_DIR"); s) {
		out.textureDir = s; }
	if (s = getenv("RQDQ__VIEWER__MESH_DIR"); s) {
		out.meshDir = s; }
	if (s = getenv("RQDQ__VIEWER__FULLSCREEN"); s) {
		out.fullScreen = boolify(s); }
	if (s = getenv("RQDQ__VIEWER__OUTPUT_SIZE"); s) {
		out.outputSizeInPx = stoivec2(s); }
	if (s = getenv("RQDQ__VIEWER__TILE_SIZE"); s) {
		out.tileSizeInBlocks = stoivec2(s); }
	return out; }


auto GetArgConfig(int argc, char** argv) -> AppConfig {
	using rclt::ConsumePrefix;
	AppConfig out;
	std::vector<std::string> np{};
	for (int i=1; i<argc; ++i) {
		std::string tmp{argv[i]};
		if (ConsumePrefix(tmp, "-c") || ConsumePrefix(tmp, "--config=")) {
			out.configPath = tmp; }
		else if (ConsumePrefix(tmp, "-d") || ConsumePrefix(tmp, "--debug")) {
			out.debug = true; }
		else if (ConsumePrefix(tmp, "-C") || ConsumePrefix(tmp, "--concurrency=")) {
			out.concurrency = stoi(tmp); }
		else if (ConsumePrefix(tmp, "-n") || ConsumePrefix(tmp, "--nodes=")) {
			np.push_back(tmp); }
		else if (ConsumePrefix(tmp, "-l") || ConsumePrefix(tmp, "--latency=")) {
			out.latencyInFrames = stoi(tmp); }
		else if (ConsumePrefix(tmp, "--texture-dir=")) {
			out.textureDir = tmp; }
		else if (ConsumePrefix(tmp, "--mesh-dir=")) {
			out.meshDir = tmp; }
		else if (ConsumePrefix(tmp, "-f") || ConsumePrefix(tmp, "--fullscreen")) {
			out.fullScreen = true; }
		else if (ConsumePrefix(tmp, "-m") || ConsumePrefix(tmp, "--mode=")) {
			out.outputSizeInPx = stoivec2(tmp); }
		else if (ConsumePrefix(tmp, "-t") || ConsumePrefix(tmp, "--tile=")) {
			out.tileSizeInBlocks = stoivec2(tmp); }
		else {
			std::cerr << "warning: unknown arg \"" << tmp << "\"\n";}}
	if (!np.empty()) {
		out.nodePath = np; }
	return out; }


auto GetFileConfig(const std::string& fn) -> AppConfig {
	JSONFile f(fn);
	auto root = f.GetRoot();
	AppConfig out;
	std::vector<std::string> np;

	if (auto jv = jv_find(root, "debug", JSON_TRUE)) {
		out.debug = true; }
	if (auto jv = jv_find(root, "debug", JSON_FALSE)) {
		out.debug = false; }

	if (auto jv = jv_find(root, "telemetryScale", JSON_NUMBER)) {
		out.telemetryScale = static_cast<int>(jv->toNumber()); }
	
	if (auto jv = jv_find(root, "nice", JSON_TRUE)) {
		out.nice = true; }
	if (auto jv = jv_find(root, "nice", JSON_FALSE)) {
		out.nice = false; }

	if (auto jv = jv_find(root, "concurrency", JSON_NUMBER)) {
		out.concurrency = static_cast<int>(jv->toNumber()); }

	if (auto jv = jv_find(root, "scene", JSON_STRING)) {
		np = { { jv->toString() } }; }
	if (auto jv = jv_find(root, "scene", JSON_ARRAY)) {
       for (const auto item : *jv) {
          np.emplace_back(item->value.toString()); } }

	if (auto jv = jv_find(root, "latency", JSON_NUMBER)) {
		out.latencyInFrames = static_cast<int>(jv->toNumber()); }

	if (auto jv = jv_find(root, "textureDir", JSON_STRING)) {
		out.textureDir = jv->toString(); }

	if (auto jv = jv_find(root, "meshDir", JSON_STRING)) {
		out.meshDir = jv->toString(); }

	if (auto jv = jv_find(root, "fullScreen", JSON_TRUE)) {
		out.fullScreen = true; }
	if (auto jv = jv_find(root, "fullScreen", JSON_FALSE)) {
		out.fullScreen = false; }

	return out; }


}  // close package namespace
}  // close enterprise namespace
