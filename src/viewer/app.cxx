#include "app.hxx"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

#ifdef ENABLE_MUSIC
#include "src/ral/ralio/ralio_audio_controller.hxx"
#include "src/ral/rals/rals_sync_controller.hxx"
#endif
#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rcls/rcls_smoothedintervaltimer.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rgl/rglr/rglr_display_mode.hxx"
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

#include "3rdparty/fmt/include/fmt/format.h"
#include "3rdparty/fmt/include/fmt/printf.h"
#include "3rdparty/gason/gason.h"
#include "3rdparty/pixeltoaster/PixelToaster.h"
#include <Windows.h>

namespace rqdq {
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

std::vector<DisplayMode> modelist = {
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


class Application::impl : public PixelToaster::Listener {

	PixelToaster::Display display_;
	rglv::MeshStore meshStore_;
	TextureStore textureStore_;
	ProPrinter pp_;
	rcls::SmoothedIntervalTimer refreshTime_;
	PixelToaster::Timer renderTime_;
	double lastRenderTime_;
	PixelToaster::Timer wallClock_;

	// BEGIN debugger state
	bool shouldQuit_ = false;
	int runtimeInFrames_{ 0 };
	int taskSize_ = 6;
	ivec2 tile_dim{ 12, 4 };

	int vis_scale = 30;
	bool debug_mode = false;
	bool show_shader_threads = false;
	bool isPaused_ = false;
	bool runFullScreen_ = false;
	bool nice_ = true;

	bool mouseCaptured_ = false;
	bool reset_mouse_next_frame = false;
	rmlv::vec2 mousePositionInPx_{};

	bool measuring = false;
	bool start_measuring = false;
	bool scanning = false;
	bool start_scanning = false;
	bool stop_scanning = false;
	double scan_min_value;
	ivec2 scan_min_dim;

	std::vector<double> measurementSamples_;
	BenchStat last_stats;
	bool show_stats = false;
	DisplayMode cur_mode{ 0, 0 };
	DisplayMode change_mode = modelist[3];
	bool runningFullScreen_ = false;
	bool show_mode_list = false;
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
	void Run() {
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
			return; }


#ifdef ENABLE_MUSIC
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

		const double rowsPerSecond = (double(soundtrack_.tempoInBeatsPerMinute) / 60) * syncConfig_.precisionInRowsPerBeat;
		const int songDurationInRows = soundtrack_.durationInSeconds * rowsPerSecond;

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

				if (mouseCaptured_ && reset_mouse_next_frame) {
					reset_mouse_next_frame = false;
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
				globalsNode_->Upsert("windowSize", rmlv::vec2(float(cur_mode.width_in_pixels), float(cur_mode.height_in_pixels)));
				globalsNode_->Upsert("tileSize", rmlv::vec2(float(tile_dim.x), float(tile_dim.y)));
				globalsNode_->Upsert("windowAspect", float(cur_mode.width_in_pixels) / float(cur_mode.height_in_pixels));

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

	void SetFullScreen(bool value) {
		runFullScreen_ = value; }

	void SetNice(bool value) {
		nice_ = value; }

private:
	auto defaultKeyHandlers() const -> bool override {
		return false; }

	void onKeyPressed(PixelToaster::DisplayInterface& display, PixelToaster::Key key) override {
		if (!debug_mode) {
			if (key == Key::F1) {
				debug_mode = !debug_mode;
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
		case Key::OpenBracket:
			taskSize_ = max(4, taskSize_ - 1);
			tile_dim = ivec2{ taskSize_, taskSize_ };
			break;
		case Key::CloseBracket:
			taskSize_ = min(taskSize_ + 1, 128);
			tile_dim = ivec2{ taskSize_, taskSize_ };
			break;
		case Key::F1:
			debug_mode = !debug_mode;
			break;
		case Key::F2:
			show_shader_threads = !show_shader_threads;
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
			vis_scale += max(vis_scale / 10, 1);
			break;
		case Key::Comma:
			vis_scale = max(1, vis_scale - (vis_scale / 10));
			break;
		case Key::R:
			start_measuring = true;
			break;
		case Key::C:
			show_stats = false;
			break;
		case Key::F:
			runFullScreen_ = !runFullScreen_;
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
			if (!measuring && !scanning) {
				start_scanning = true; }
			else if (scanning) {
				stop_scanning = true; }
			break;
	*/
		default:
	//		cout << "key was " << key << endl;
			if (show_mode_list && (key >= Key::One && key < (Key::One + modelist.size()))) {
				int modeidx = key - Key::One;
				change_mode = modelist[modeidx]; }}}

	void onKeyDown(PixelToaster::DisplayInterface&, PixelToaster::Key key) override {
		switch (key) {
		case Key::Escape:
			shouldQuit_ = true;
			break;
		case Key::M:
			show_mode_list = true;
			break;
		case Key::Shift:
			keys_shifted = true;
			break;
		default: break; }}

	void onKeyUp(PixelToaster::DisplayInterface&, PixelToaster::Key key) override {
		switch (key) {
		case Key::M:
			show_mode_list = false;
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
			reset_mouse_next_frame = true; }}

	void onMouseWheel(PixelToaster::DisplayInterface&, PixelToaster::Mouse, short wheel_amount) override {
		int ticks = wheel_amount / 120;
		camera_.OnMouseWheel(ticks); }

	void DrawUI(TrueColorCanvas& canvas) {
		auto renderTimeInMillis = lastRenderTime_ * 1000.0; // renderTime_.Get();
		auto refreshTimeInMillis = refreshTime_.Get();

		if (show_mode_list) {
			int idx = 1;
			int top = canvas.height() / 2;
			for (const auto& mode : modelist) {
				stringstream ss;
				ss << idx << ": " << mode.width_in_pixels << "x" << mode.height_in_pixels;
				if (cur_mode == mode) {
					ss << " (current)"; }
				pp_.write(ss.str(), 16, top, canvas);
				top += 10;
				idx += 1; } }

		if (start_measuring) {
			measuring = true;
			start_measuring = false;
			show_stats = false;
			measurementSamples_.clear(); }

		if (measuring) {
			if (measurementSamples_.size() == MEASUREMENT_SAMPLESIZE_IN_FRAMES) {
				measuring = false;
				last_stats = CalcStat(measurementSamples_, MEASUREMENT_DISCARD);
				show_stats = true; }
			else {
				measurementSamples_.push_back(renderTimeInMillis);
				stringstream ss;
				ss << "measuring, " << measurementSamples_.size() << " / " << MEASUREMENT_SAMPLESIZE_IN_FRAMES;
				pp_.write(ss.str(), 16, 100, canvas); } }

		if (start_scanning) {
			scanning = true;
			start_scanning = false;
			tile_dim = ivec2{ 2, 2 };
			scan_min_dim = ivec2{ 2, 2 };
			scan_min_value = 10000.0;
			measurementSamples_.clear(); }

		if (stop_scanning) {
			stop_scanning = false;
			scanning = false;
			tile_dim = scan_min_dim; }

		if (scanning) {
			{
				int top = 100;
				stringstream ss;
				ss << "probing for fastest tile dimensions: " << (((tile_dim.y - 1) * 16) + tile_dim.x - 1) << " / " << (16 * 16) << "   ";
				pp_.write(ss.str(), 16, top, canvas);  top += 10;
				ss.str("");
				ss << "                     fastest so far: " << scan_min_dim.x << "x" << scan_min_dim.y << "   ";
				pp_.write(ss.str(), 16, top, canvas);  top += 10;
				ss.str("");
				ss << "          press s to stop            ";
				pp_.write(ss.str(), 16, top, canvas);  top += 10; }
			if (measurementSamples_.size() == SCAN_SAMPLESIZE_IN_FRAMES) {
				last_stats = CalcStat(std::vector<double>(begin(measurementSamples_) + 60, end(measurementSamples_)), MEASUREMENT_DISCARD);
				measurementSamples_.clear();
				if (last_stats.avg < scan_min_value) {
					scan_min_value = last_stats.avg;
					scan_min_dim = tile_dim; }
				tile_dim.x += 2;
				if (tile_dim.x > SCAN_SIZE_LIMIT_IN_TILES.x) {
					tile_dim.x = 2;
					tile_dim.y += 2; }
				if (tile_dim.y > SCAN_SIZE_LIMIT_IN_TILES.y) {
					scanning = false;
					tile_dim = scan_min_dim; } }
			else {
				measurementSamples_.push_back(renderTimeInMillis); } }

		if (debug_mode) {
			double fps = 1.0 / (refreshTimeInMillis / 1000.0);
			auto s = fmt::sprintf("% 6.2f ms, fps: %.0f", renderTimeInMillis, fps);
			pp_.write(s, 16, 16, canvas); }

		if (debug_mode) {
			stringstream ss;
			ss << "tile size: " << tile_dim.x << "x" << tile_dim.y;
			ss << ", ";
			ss << "visu scale: " << vis_scale;
			if (isPaused_) {
				ss << "   PAUSED"; }
			pp_.write(ss.str(), 16, 27, canvas); }

		if (debug_mode) {
			stringstream ss;
			ss << "F1 debug         l  toggle srgb   [&] tile size    ,&. vis scale       ";
			pp_.write(ss.str(), 16, -32, canvas);
			ss.str("");
			ss << " p toggle pause  m  change mode    r  measure       n  wasd capture    ";
			pp_.write(ss.str(), 16, -42, canvas);
			ss.str("");
			ss << " f fullscreen   F2  show tiles     g  dblbuf        shift -&+ grid size";
			pp_.write(ss.str(), 16, -52, canvas); }
		else if (runtimeInFrames_ < (5 * 60)) {
			stringstream ss;
			int top = canvas.height() - 11;
			ss << "F1 debug";
			pp_.write(ss.str(), 0, top, canvas); }

		if (debug_mode) {
			render_jobsys(20, 40, float(vis_scale), canvas); }

		if (debug_mode && show_stats) {
			using fmt::format_to, fmt::to_string;
			int top = canvas.height() / 2;
			pp_.write("   min    25th     med    75th     max    mean    sdev", 32, top, canvas);
			top += 10;
			fmt::memory_buffer out;
			format_to(out, "{: 6.2f}  ", last_stats.min);
			format_to(out, "{: 6.2f}  ", last_stats.p25);
			format_to(out, "{: 6.2f}  ", last_stats.med);
			format_to(out, "{: 6.2f}  ", last_stats.p75);
			format_to(out, "{: 6.2f}  ", last_stats.max);
			format_to(out, "{: 6.2f}  ", last_stats.avg);
			format_to(out, "{: 6.2f}  ", last_stats.std);
			pp_.write(to_string(out), 32, top, canvas);
			top += 10;
			pp_.write(string("press C to clear"), 32, top, canvas); } }

	void PrepareBuiltInNodes() {
		globalsNode_ = make_shared<MultiValueNode>("globals", InputList());
		appNodes_.emplace_back(globalsNode_);

		syncNode_ = make_shared<MultiValueNode>("sync", InputList());
		appNodes_.emplace_back(syncNode_);

		uiCameraNode_ = make_shared<UICamera>("uiCamera", InputList(), camera_);
		appNodes_.emplace_back(uiCameraNode_); }

	void MaybeUpdateDisplay() {
		if (cur_mode != change_mode || runningFullScreen_ != runFullScreen_) {
			cur_mode = change_mode;
			runningFullScreen_ = runFullScreen_;
			display_.close();
			display_.open("rqdq 2018",
						  cur_mode.width_in_pixels,
						  cur_mode.height_in_pixels,
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
			std::cerr << fmt::sprintf("scene compiled in %.2fms\n", elapsed);
			return true; }

		// new node-set failed, re-link the previous set
		if (!Link(nodes_)) {
			std::cerr << "re-link old nodes failed!\n";
			std::exit(1); }
		return false; }};


Application::Application() :impl_(std::make_unique<impl>()) {}
Application::~Application() = default;
Application& Application::operator=(Application&&) noexcept = default;

Application& Application::SetNice(bool value) {
	impl_->SetNice(value);
	return *this;}

Application& Application::SetFullScreen(bool value) {
	impl_->SetFullScreen(value);
	return *this;}

Application& Application::Run() {
	impl_->Run();
	return *this;}


}  // namespace rqv
}  // namespace rqdq
