#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclt/rclt_util.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/viewer/resource.h"
#include "src/viewer/app.hxx"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <fmt/printf.h>
#include "3rdparty/gason/gason.h"
#include <Windows.h>


auto LevelToThreads(int level) -> int {
	const auto maxThreads = int(std::thread::hardware_concurrency());
	if (level == 0) {
		return maxThreads; }
	else if (level < 0) {
		return maxThreads + level; }
	else {
		return std::clamp(level, 0, maxThreads); }}


int main(int argc, char** argv) {
	using namespace rqdq;
	using namespace rqdq::rclx;
	using namespace rqdq::rqv;
	namespace jobsys = rqdq::rclmt::jobsys;
	namespace framepool = rqdq::rclma::framepool;

	auto argConfig = GetArgConfig(argc, argv);
	auto envConfig = GetEnvConfig();
	auto configPath = Merge(envConfig, argConfig).configPath.value_or("data/viewer_config.json");
	auto fileConfig = configPath != "null" ? GetFileConfig(configPath) : PartialAppConfig{};
	auto allConfig = Merge(fileConfig, Merge(envConfig, argConfig));
	int threads = LevelToThreads(allConfig.concurrency.value_or(0));

	jobsys::telemetryEnabled = true;
	jobsys::init(threads);
	framepool::Init();

#ifdef NDEBUG
	try {
#endif
		auto app = Application(allConfig);
		app.Run();
#ifdef NDEBUG
	} catch (const std::exception& err) {
 		std::cout << "exception: " << err.what() << std::endl; }
	catch (...) {
		std::cout << "caught unknown exception\n"; }
#endif

	jobsys::stop();
	jobsys::join();
	return 0; }


struct SplashDialogResult {
	bool start{false};
	bool fullScreen{false};
	// int wantednMode = -1;
	bool force720p{false}; };


class SplashDialog {

	HWND window_{0};
	SplashDialogResult result_{};

public:
	auto Create(HINSTANCE hInst) -> SplashDialog& {
		window_ = CreateDialogParamW(hInst, MAKEINTRESOURCE(IDD_SPLASH), nullptr, DialogProcJmp, reinterpret_cast<LPARAM>(this));
		return *this; }

	auto Show(int nCmdShow) -> SplashDialog& {
		ShowWindow(window_, nCmdShow);
		return *this; }

	auto RunUntilClosed() -> SplashDialog& {
		MSG msg;
		BOOL ret;
		while ((ret = GetMessageW(&msg, nullptr, 0, 0)) != 0) {
			if (ret == -1) {
				throw std::runtime_error("GetMessage error"); }
			if (IsDialogMessageW(window_, &msg) == 0) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg); }}
		return *this; }

	auto Result() {
		return result_; }

private:
	static
	auto CALLBACK DialogProcJmp(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
		if (uMsg == WM_INITDIALOG) {
			SetWindowLongPtr(hDlg, DWLP_USER, lParam); }
		auto* pThis = reinterpret_cast<SplashDialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
		return pThis != nullptr ? pThis->DialogProc(hDlg, uMsg, wParam, lParam) : FALSE; }
	auto DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam [[maybe_unused]]) -> INT_PTR {
		switch (uMsg) {
		case WM_INITDIALOG: {
			/*HWND combogpu = GetDlgItem(hDlg, IDC_GPU);
			for (int i = 0; i < sizeof(igpu) / sizeof(TCHAR*); i++) {
				SendMessage(combogpu, CB_ADDSTRING, 0, (LPARAM)igpu[i]); }
			SendMessage(combogpu, CB_SETCURSEL, 0, 0);*/
			SendMessage(GetDlgItem(hDlg, IDC_TDFMODE), BM_SETCHECK, 1, 0);
			SendMessage(GetDlgItem(hDlg, IDC_FULLSCREEN), BM_SETCHECK, 1, 0);
			return TRUE; }

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDCANCEL:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			case IDOK:
				result_.start = true;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE; }
			break;

		case WM_CLOSE:
			result_.fullScreen = SendMessage(GetDlgItem(hDlg, IDC_FULLSCREEN), BM_GETCHECK, 0, 0) > 0;
			result_.force720p = SendMessage(GetDlgItem(hDlg, IDC_TDFMODE), BM_GETCHECK, 0, 0) > 0;
			//run_screenmode = SendMessage( GetDlgItem(hDlg,IDC_RES), CB_GETCURSEL, 0, 0 );
			DestroyWindow(hDlg);
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE; }
		return FALSE; }};


auto WINAPI WinMain(HINSTANCE hInst, HINSTANCE h0 [[maybe_unused]], LPSTR lpCmdLine [[maybe_unused]], int nCmdShow) -> int {
	using namespace rqdq::rqv;
	namespace jobsys = rqdq::rclmt::jobsys;
	namespace framepool = rqdq::rclma::framepool;

	auto wants = SplashDialog().Create(hInst).Show(nCmdShow).RunUntilClosed().Result();

	if (!wants.start) {
		return 0;}

	PartialAppConfig config;
	config.idlePolicy = IdlePolicy::Sleep;
	config.fullScreen = wants.fullScreen;
	int threads = LevelToThreads(config.concurrency.value_or(0));
	jobsys::init(threads);
	framepool::Init();

	auto app = Application(config);
	/*
	if (want720p) {
		//hackmode_enable = 1;
		//hackmode_width = 1280;
		//hackmode_height = 720; }
	else {
		hackmode_enable = 0; }
	*/
	// app.startedFromGUI = true;  XXX for modesetting
	app.Run();
	return 0; }
