#include "src/viewer/app.hxx"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <thread>
#include <vector>

#include "src/ral/ralio/ralio_audio_controller.hxx"
#include "src/ral/rals/rals_sync_controller.hxx"
#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclr/rclr_algorithm.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/rgl/rglr/rglr_display_mode.hxx"
#include "src/rgl/rglr/rglr_pixeltoaster_util.hxx"
#include "src/rgl/rglr/rglr_profont.hxx"
#include "src/rgl/rglr/rglr_texture_store.hxx"
#include "src/rgl/rglv/rglv_camera.hxx"
#include "src/rgl/rglv/rglv_material.hxx"
#include "src/rgl/rglv/rglv_mesh_store.hxx"
#include "src/rml/rmls/rmls_bench.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/viewer/compile.hxx"
#include "src/viewer/jobsys_vis.hxx"
#include "src/viewer/node/camera.hxx"
#include "src/viewer/node/output.hxx"
#include "src/viewer/node/value.hxx"

#include "3rdparty/fmt/include/fmt/format.h"
#include "3rdparty/fmt/include/fmt/printf.h"
#include "3rdparty/gason/gason.h"
#include "3rdparty/pixeltoaster/PixelToaster.h"
#include <Windows.h>

namespace rqdq {
namespace rqv {

using namespace PixelToaster;
using namespace ralio;
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





struct SampleSmoother {
	double value = 0.0;
	void add(double a) {
		value = (value * 0.9) + (a * 0.1); }};


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


optional<Soundtrack> deserializeSoundtrack(const JsonValue& data) {
	double tempo;
	double duration = 0;
	string path;

	if (auto search = jv_find(data, "tempo", JSON_NUMBER)) {
		tempo = search->toNumber(); }
	else {
		cout << "soundtrack: missing tempo\n";
		return {}; }

	if (auto search = jv_find(data, "path", JSON_STRING)) {
		path = search->toString(); }
	else {
		cout << "soundtrack: missing path\n";
		return {}; }

	if (auto dnode = jv_find(data, "duration", JSON_OBJECT)) {
		if (auto search = jv_find(*dnode, "minutes", JSON_NUMBER)) {
			duration += search->toNumber() * 60.0; }
		if (auto search = jv_find(*dnode, "seconds", JSON_NUMBER)) {
			duration += search->toNumber(); }
		if (duration == 0) {
			cout << "soundtrack: duration node should indicate duration in minutes and/or seconds\n";
			return {}; }}
	else {
		cout << "soundtrack: missing duration node\n";
		return {}; }

	return Soundtrack{ path, tempo, duration };}


struct SyncConfig {
	int precisionInRowsPerBeat;
	vector<string> trackNames; };


optional<SyncConfig> deserializeSyncConfig(const JsonValue& data) {
	SyncConfig sc;

	if (auto search = jv_find(data, "precision", JSON_NUMBER)) {
		sc.precisionInRowsPerBeat = int(search->toNumber()); }
	else {
		cout << "syncConfig: missing precision\n";
		return {}; }

	if (auto search = jv_find(data, "tracks", JSON_ARRAY)) {
		for (const auto item : *search) {
			sc.trackNames.push_back(item->value.toString()); }}
	if (sc.trackNames.empty()) {
		cout << "syncConfig: warning: no track names configured\n"; }
	return sc; }


class Application::impl : public PixelToaster::Listener {
public:
	void run();
	void setFullScreen(bool value) { d_runFullScreen = value; }
	void setNice(bool value) { d_nice = value; }

private:
	bool defaultKeyHandlers() const;
	void onKeyPressed(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
	void onKeyDown(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
	void onKeyUp(PixelToaster::DisplayInterface& display, PixelToaster::Key key);
//	void onMouseButtonDown(PixelToaster::DisplayInterface & display, PixelToaster::Mouse mouse);
	void onMouseMove(PixelToaster::DisplayInterface & display, PixelToaster::Mouse mouse);
	void onMouseWheel(PixelToaster::DisplayInterface & display, PixelToaster::Mouse mouse, short wheel_amount);

	void drawUI(struct TrueColorCanvas&, double, double);

	void prepareBuiltInNodes();
	void maybeUpdateDisplay();
	void computeAndRenderFrame(TrueColorCanvas canvas);
	bool recompile(const JsonValue & docroot);

	PixelToaster::Display d_display;
	rglv::MeshStore d_meshStore;
	MaterialStore d_materialStore;
	TextureStore d_textureStore;
	ProPrinter pp;
	PixelToaster::Timer d_interFrameTimer;
	PixelToaster::Timer d_renderTimer;
	PixelToaster::Timer d_wallClock;

	// BEGIN debugger state
	bool d_quit = false;
	int d_runtimeInFrames{ 0 };
	int d_taskSize = 6;
	ivec2 tile_dim{ 12, 4 };

	int vis_scale = 30;
	bool debug_mode = true;
	bool show_shader_threads = false;
	bool d_isPaused = false;
	bool d_runFullScreen = false;
	bool d_nice = true;

	bool capture_mouse = false;
	bool reset_mouse_next_frame = false;
	int mouse_x_position_in_px, mouse_y_position_in_px;

	bool measuring = false;
	bool start_measuring = false;
	bool scanning = false;
	bool start_scanning = false;
	bool stop_scanning = false;
	double scan_min_value;
	ivec2 scan_min_dim;

	std::vector<double> d_measurementSamples;
	BenchStat last_stats;
	bool show_stats = false;
	DisplayMode cur_mode{ 0, 0 };
	DisplayMode change_mode = modelist[3];
	bool d_doubleBuffer = true;
	bool d_runningFullScreen = false;
	bool show_mode_list = false;
	bool keys_shifted = false;

	HandyCam d_camera;
	// END debugger state

	// current scene and built-in nodes
	NodeList d_nodes;
	NodeList d_appNodes;
	std::shared_ptr<MultiValueNode> d_globalsNode = make_shared<MultiValueNode>("globals", InputList());
	std::shared_ptr<MultiValueNode> d_syncNode = make_shared<MultiValueNode>("sync", InputList());
	std::shared_ptr<HandyCamNode> d_uiCameraNode = make_shared<HandyCamNode>("uiCamera", InputList(), d_camera);

#ifdef ENABLE_MUSIC
	// music/sync
	Soundtrack d_soundtrack;
	SyncConfig d_syncConfig;
#endif
	};


void Application::impl::run() {
	d_textureStore.load_dir("data\\textures\\");
	d_meshStore.load_dir("data\\meshes\\", d_materialStore, d_textureStore);

	prepareBuiltInNodes();

	// initial read from scene.json
	JSONFile sceneJson("data/scene.json");
	if (!sceneJson.IsValid()) {
		cout << "error while reading data/scene.json, can't continue.\n";
		return; }
	if (auto success = recompile(sceneJson.GetRoot()); !success) {
		cout << "compile failed, can't continue.\n";
		return; }

	JSONFile config_json("data/viewer_config.json");

#ifdef ENABLE_MUSIC
	if (auto jv = jv_find(config_json.GetRoot(), "soundtrack", JSON_OBJECT)) {
		if (auto result = deserializeSoundtrack(*jv)) {
			d_soundtrack = result.value(); }
		else {
			return; }}
	else {
		cout << "compiled with ENABLE_MUSIC but soundtrack config not found, can't continue\n";
		return; }

	if (auto jv = jv_find(config_json.GetRoot(), "sync", JSON_OBJECT)) {
		if (auto result = deserializeSyncConfig(*jv)) {
			d_syncConfig = result.value(); }
		else {
			return; }}
	else {
		cout << "compiled with ENABLE_MUSIC but sync config not found, can't continue\n";
		return; }

	AudioController audioController;

	auto result = audioController.CreateStream(d_soundtrack.path);
	if (!result.has_value()) {
		cout << "failed to load " << d_soundtrack.path << ", can't continue.\n";
		return; }

	auto soundtrack = std::move(result.value());

	const double rowsPerSecond = (double(d_soundtrack.tempoInBeatsPerMinute) / 60) * d_syncConfig.precisionInRowsPerBeat;
	const int songDurationInRows = d_soundtrack.durationInSeconds * rowsPerSecond;

	rals::SyncController syncController(std::string("data/sync"), soundtrack, rowsPerSecond);
#ifndef SYNC_PLAYER
	syncController.Connect();
#endif
	for (const auto& name : d_syncConfig.trackNames) {
		syncController.AddTrack(name); }

	audioController.Start();
	soundtrack.Play();
#endif //ENABLE_MUSIC

	if (!d_nice) { jobsys::work_start();}

	SampleSmoother renderTimeInMillis;
	SampleSmoother frameTimeInMillis;
	try {
		while (!d_quit) {
			maybeUpdateDisplay();

			if (sceneJson.IsOutOfDate()) {
				jobsys::_sleep(100);
				sceneJson.Refresh();
				if (sceneJson.IsValid()) {
					recompile(sceneJson.GetRoot()); }}

			if (capture_mouse && reset_mouse_next_frame) {
				reset_mouse_next_frame = false;
				d_display.center_mouse(); }

			d_renderTimer.reset();
			auto frame = Frame(d_display);
			auto canvas = frame.canvas();

			if (d_nice) jobsys::work_start();
			jobsys::reset();
			framepool::Reset();

#ifdef ENABLE_MUSIC
			double musicPositionInRows = syncController.GetPositionInRows();
			syncController.ForEachValue(musicPositionInRows, [this](const auto& name, auto value) {
				d_syncNode->upsert(name, float(value)); });

#ifdef SYNC_PLAYER
			// terminate when song is over
			if (musicPositionInRows > songDurationInRows) {
				d_quit = true; }  // end of soundtrack == end of demo
#else
			// host send-and-receive
			if (syncController.Update((int)floor(musicPositionInRows))) {
				syncController.Connect(); }
#endif
#endif // ENABLE_MUSIC

			// update globals
			d_globalsNode->upsert("wallclock", d_isPaused ? float(0) : float(d_wallClock.time()));
			d_globalsNode->upsert("displayWidth", float(cur_mode.width_in_pixels));
			d_globalsNode->upsert("displayHeight", float(cur_mode.height_in_pixels));
			d_globalsNode->upsert("displayAspect", float(cur_mode.width_in_pixels) / float(cur_mode.height_in_pixels));

			computeAndRenderFrame(canvas);
			if (d_nice) { jobsys::work_end();}

#ifdef ENABLE_MUSIC
			audioController.FillBuffers();  // decrease chance of missing vsync
#endif

			renderTimeInMillis.add(d_renderTimer.time() * 1000.0);
			frameTimeInMillis.add(d_interFrameTimer.delta() * 1000.0);
			drawUI(canvas, renderTimeInMillis.value, frameTimeInMillis.value);
			d_runtimeInFrames++; }}

	catch (WindowClosed) {
		cout << "window was closed" << endl; }
	catch (WrongFormat) {
		cout << "framebuffer not rgba8888" << endl; }

	if (!d_nice) { jobsys::work_end();}

	cout << "terminated.\n";
#ifdef ENABLE_MUSIC
#ifndef SYNC_PLAYER
	syncController.SaveTracks();
#endif
#endif
}


bool Application::impl::defaultKeyHandlers() const {
	return false; }


void Application::impl::onKeyPressed(DisplayInterface& display, Key key) {
	switch (key) {
	case Key::OpenBracket:
		d_taskSize = max(4, d_taskSize - 1);
		tile_dim = ivec2{ d_taskSize, d_taskSize };
		break;
	case Key::CloseBracket:
		d_taskSize = min(d_taskSize + 1, 128);
		tile_dim = ivec2{ d_taskSize, d_taskSize };
		break;
	case Key::F1:
		debug_mode = !debug_mode;
		break;
	case Key::F2:
		show_shader_threads = !show_shader_threads;
		break;
	case Key::P:
		d_camera.print();
		d_isPaused= !d_isPaused;
		break;
	case Key::W: d_camera.moveForward(); break;
	case Key::S: d_camera.moveBackward(); break;
	case Key::A: d_camera.moveLeft(); break;
	case Key::D: d_camera.moveRight(); break;
	case Key::E: d_camera.moveUp();  break;
	case Key::Q: d_camera.moveDown();  break;
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
		d_runFullScreen = !d_runFullScreen;
		break;
	case Key::G:
		d_doubleBuffer = !d_doubleBuffer;
		break;
	case Key::N:
		capture_mouse = !capture_mouse;
		if (capture_mouse) {
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


void Application::impl::onKeyDown(DisplayInterface& display, Key key) {
	switch (key) {
	case Key::M:
		show_mode_list = true;
		break;
	case Key::Shift:
		keys_shifted = true;
		break;
	default: break; }}


void Application::impl::onKeyUp(DisplayInterface& display, Key key) {
	switch (key) {
	case Key::M:
		show_mode_list = false;
		break;
	case Key::Shift:
		keys_shifted = false;
		break;
	default: break; }}


void Application::impl::onMouseMove(DisplayInterface& display, Mouse mouse) {
	mouse_x_position_in_px = int(mouse.x);
	mouse_y_position_in_px = int(mouse.y);
	float dmx = (display.width() / 2) - mouse.x;
	float dmy = (display.height() / 2) - mouse.y;
	if (capture_mouse) {
		//cout << "mm(" << dmx << ", " << dmy << ")" << endl;
		d_camera.onMouseMove({ dmx, dmy });
		reset_mouse_next_frame = true; }}


void Application::impl::onMouseWheel(DisplayInterface& display, Mouse mouse, short wheel_amount) {
	int ticks = wheel_amount / 120;
	d_camera.adjustZoom(ticks); }


void Application::impl::drawUI(
	TrueColorCanvas& canvas,
	double renderTimeInMillis,
	double frameTimeInMillis
) {
	if (show_mode_list) {
		int idx = 1;
		int top = canvas.height() / 2;
		for (const auto& mode : modelist) {
			stringstream ss;
			ss << idx << ": " << mode.width_in_pixels << "x" << mode.height_in_pixels;
			if (cur_mode == mode) {
				ss << " (current)"; }
			pp.write(ss.str(), 16, top, canvas);
			top += 10;
			idx += 1; } }

	if (start_measuring) {
		measuring = true;
		start_measuring = false;
		show_stats = false;
		d_measurementSamples.clear(); }

	if (measuring) {
		if (d_measurementSamples.size() == MEASUREMENT_SAMPLESIZE_IN_FRAMES) {
			measuring = false;
			last_stats = calc_stat(d_measurementSamples, MEASUREMENT_DISCARD);
			show_stats = true; }
		else {
			d_measurementSamples.push_back(renderTimeInMillis);
			stringstream ss;
			ss << "measuring, " << d_measurementSamples.size() << " / " << MEASUREMENT_SAMPLESIZE_IN_FRAMES;
			pp.write(ss.str(), 16, 100, canvas); } }

	if (start_scanning) {
		scanning = true;
		start_scanning = false;
		tile_dim = ivec2{ 2, 2 };
		scan_min_dim = ivec2{ 2, 2 };
		scan_min_value = 10000.0;
		d_measurementSamples.clear(); }

	if (stop_scanning) {
		stop_scanning = false;
		scanning = false;
		tile_dim = scan_min_dim; }

	if (scanning) {
		{
			int top = 100;
			stringstream ss;
			ss << "probing for fastest tile dimensions: " << (((tile_dim.y - 1) * 16) + tile_dim.x - 1) << " / " << (16 * 16) << "   ";
			pp.write(ss.str(), 16, top, canvas);  top += 10;
			ss.str("");
			ss << "                     fastest so far: " << scan_min_dim.x << "x" << scan_min_dim.y << "   ";
			pp.write(ss.str(), 16, top, canvas);  top += 10;
			ss.str("");
			ss << "          press s to stop            ";
			pp.write(ss.str(), 16, top, canvas);  top += 10; }
		if (d_measurementSamples.size() == SCAN_SAMPLESIZE_IN_FRAMES) {
			last_stats = calc_stat(std::vector<double>(d_measurementSamples.begin() + 60, d_measurementSamples.end()), MEASUREMENT_DISCARD);
			d_measurementSamples.clear();
			if (last_stats._mean < scan_min_value) {
				scan_min_value = last_stats._mean;
				scan_min_dim = tile_dim; }
			tile_dim.x += 2;
			if (tile_dim.x > SCAN_SIZE_LIMIT_IN_TILES.x) {
				tile_dim.x = 2;
				tile_dim.y += 2; }
			if (tile_dim.y > SCAN_SIZE_LIMIT_IN_TILES.y) {
				scanning = false;
				tile_dim = scan_min_dim; } }
		else {
			d_measurementSamples.push_back(renderTimeInMillis); } }

	if (debug_mode) {
		double fps = 1.0 / (frameTimeInMillis / 1000.0);
		auto s = fmt::sprintf("% 6.2f ms, fps: %.0f", renderTimeInMillis, fps);
		pp.write(s, 16, 16, canvas); }

	if (debug_mode) {
		stringstream ss;
		ss << "tile size: " << tile_dim.x << "x" << tile_dim.y;
		ss << ", ";
		ss << "visu scale: " << vis_scale;
		if (d_isPaused) {
			ss << "   PAUSED"; }
		pp.write(ss.str(), 16, 27, canvas); }

	if (debug_mode) {
		stringstream ss;
		ss << "F1 debug         l  toggle srgb   [&] tile size    ,&. vis scale       ";
		pp.write(ss.str(), 16, -32, canvas);
		ss.str("");
		ss << " p toggle pause  m  change mode    r  measure       n  wasd capture    ";
		pp.write(ss.str(), 16, -42, canvas);
		ss.str("");
		ss << " f fullscreen   F2  show tiles     g  dblbuf        shift -&+ grid size";
		pp.write(ss.str(), 16, -52, canvas); }
	else if (d_runtimeInFrames < (5 * 60)) {
		stringstream ss;
		int top = canvas.height() - 11;
		ss << "F1 debug";
		pp.write(ss.str(), 0, top, canvas); }

	if (debug_mode) {
		render_jobsys(20, 40, float(vis_scale), canvas); }

	if (debug_mode && show_stats) {
		using fmt::format_to, fmt::to_string;
		int top = canvas.height() / 2;
		pp.write("   min    25th     med    75th     max    mean    sdev", 32, top, canvas);
		top += 10;
		fmt::memory_buffer out;
		format_to(out, "{: 6.2f}  ", last_stats._min);
		format_to(out, "{: 6.2f}  ", last_stats._25th);
		format_to(out, "{: 6.2f}  ", last_stats._med);
		format_to(out, "{: 6.2f}  ", last_stats._75th);
		format_to(out, "{: 6.2f}  ", last_stats._max);
		format_to(out, "{: 6.2f}  ", last_stats._mean);
		format_to(out, "{: 6.2f}  ", last_stats._sdev);
		pp.write(to_string(out), 32, top, canvas);
		top += 10;
		pp.write(string("press C to clear"), 32, top, canvas); } }


void Application::impl::prepareBuiltInNodes() {
	d_globalsNode = make_shared<MultiValueNode>("globals", InputList());
	d_appNodes.push_back(d_globalsNode);

	d_syncNode = make_shared<MultiValueNode>("sync", InputList());
	d_appNodes.push_back(d_uiCameraNode);

	d_uiCameraNode = make_shared<HandyCamNode>("uiCamera", InputList(), d_camera);
	d_appNodes.push_back(d_syncNode); }


void Application::impl::maybeUpdateDisplay() {
	if (cur_mode != change_mode || d_runningFullScreen != d_runFullScreen) {
		cur_mode = change_mode;
		d_runningFullScreen = d_runFullScreen;
		d_display.close();
		d_display.open("rqdq 2018",
		               cur_mode.width_in_pixels,
		               cur_mode.height_in_pixels,
		               d_runningFullScreen ? Output::Fullscreen : Output::Windowed,
		               Mode::TrueColor);
		d_display.listener(this);
		d_display.zoom(-1.0f); }}  // auto-scale based on windows' scaling factor




void Application::impl::computeAndRenderFrame(TrueColorCanvas canvas) {
	auto rootJob = jobsys::make_job(jobsys::noop);

	const auto match = rclr::find_if(d_nodes, [](const auto node) { return node->name == "nRender"; });
	if (match != d_nodes.end()) {
		OutputNode* outputNode = static_cast<OutputNode*>(match->get());
		rclr::for_each(d_nodes, [](auto& node) { node->reset(); });
		compute_indegrees_from(outputNode);
		outputNode->indegree_wait = 1;
		outputNode->set_tile_dim(tile_dim);
		outputNode->set_double_buffer(d_doubleBuffer);
		outputNode->set_output_canvas(&canvas);
		outputNode->add_link(rootJob);
		outputNode->run(); }
	else {
		cout << "output node \"nRender\" not found\n"; }

	jobsys::wait(rootJob); }


bool Application::impl::recompile(const JsonValue& docroot) {
	PixelToaster::Timer compileTime;
	NodeList newNodes;
	bool success;
	std::tie(success, newNodes) = compile(docroot, d_meshStore);
	if (success) {
		std::copy(d_appNodes.begin(), d_appNodes.end(), std::back_inserter(newNodes));
		success = link(newNodes);
		if (success) {
			auto elapsed = compileTime.delta() * 1000.0;
			d_nodes = newNodes;
			cout << fmt::sprintf("scene compiled in %.2fms\n", elapsed);
			return true; }
		else {
			cout << "link failed, scene not updated!\n"; }}
	else {
		cout << "compile failed, scene not updated!\n"; }
	return false; }


Application::Application() :d_pImpl(std::make_unique<impl>()) {}
Application::~Application() = default;
Application& Application::operator=(Application&&) = default;

Application& Application::setNice(bool value) {
	d_pImpl->setNice(value);
	return *this;}

Application& Application::setFullScreen(bool value) {
	d_pImpl->setFullScreen(value);
	return *this;}

Application& Application::run() {
	d_pImpl->run();
	return *this;}


}  // namespace rqv
}  // namespace rqdq
