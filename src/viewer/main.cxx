#include <thread>

#include "src/rcl/rclma/rclma_framepool.hxx"
#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rclx/rclx_gason_util.hxx"
#include "src/rcl/rclx/rclx_jsonfile.hxx"
#include "src/viewer/resource.h"
#include "src/viewer/app.hxx"

#include <fmt/format.h>
#include <fmt/printf.h>
#include "3rdparty/gason/gason.h"
#include <Windows.h>

int main() {
	using namespace rqdq::rclx;
	using namespace rqdq::rqv;
	namespace jobsys = rqdq::rclmt::jobsys;
	namespace framepool = rqdq::rclma::framepool;

	JSONFile config_json("data/viewer_config.json");
	int threads = std::thread::hardware_concurrency();

	if (auto jv = jv_find(config_json.GetRoot(), "concurrency", JSON_NUMBER)) {
		const auto concurrency = int(jv->toNumber());
		if (concurrency <= 0) {
			threads = int(std::thread::hardware_concurrency()) + concurrency; }
		else {
			threads = concurrency; } }

	jobsys::init(threads); // threads);
	framepool::Init();

	try {
		auto app = Application();
		if (auto jv = jv_find(config_json.GetRoot(), "nice", JSON_TRUE)) {
			app.SetNice(true); }
		else if (auto jv = jv_find(config_json.GetRoot(), "nice", JSON_FALSE)) {
			app.SetNice(false); }
		app.Run(); }
	catch (const std::exception& err) {
		std::cout << "exception: " << err.what() << std::endl; }
	catch (...) {
		std::cout << "caught unknown exception\n"; }

	jobsys::stop();
	jobsys::join();
	return 0; }


struct SplashDialog {
	bool demoWillStart = false;
	bool wantFullScreen = false;
	bool want720p = false;
	// int wantednMode = -1;
	HWND d_hDlg;

	SplashDialog& create(HINSTANCE hInst) {
		d_hDlg = CreateDialogParamW(hInst, MAKEINTRESOURCE(IDD_SPLASH), 0, DialogProcThunk, reinterpret_cast<LPARAM>(this));
		return *this; }

	SplashDialog& show(int nCmdShow) {
		ShowWindow(d_hDlg, nCmdShow);
		return *this; }

	SplashDialog& runUntilClosed() {
		MSG msg;
		BOOL ret;
		while ((ret = GetMessageW(&msg, 0, 0, 0)) != 0) {
			if (ret == -1) {
				throw std::runtime_error("GetMessage error"); }
			if (!IsDialogMessageW(d_hDlg, &msg)) {
				TranslateMessage(&msg);
				DispatchMessageW(&msg); }}
		return *this; }

	static INT_PTR CALLBACK DialogProcThunk(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		if (uMsg == WM_INITDIALOG) {
			SetWindowLongPtr(hDlg, DWLP_USER, lParam); }
		SplashDialog* pThis = reinterpret_cast<SplashDialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
		return pThis ? pThis->DialogProc(hDlg, uMsg, wParam, lParam) : FALSE; }

	INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		switch (uMsg) {
		case WM_INITDIALOG: {
			/*HWND combogpu = GetDlgItem(hDlg, IDC_GPU);
			for (int i = 0; i < sizeof(igpu) / sizeof(TCHAR*); i++) {
				SendMessage(combogpu, CB_ADDSTRING, 0, (LPARAM)igpu[i]); }
			SendMessage(combogpu, CB_SETCURSEL, 0, 0);*/
			SendMessage(GetDlgItem(hDlg, IDC_TDFMODE), BM_SETCHECK, 1, 0);
			SendMessage(GetDlgItem(hDlg, IDC_FULLSCREEN), BM_SETCHECK, 1, 0); }
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDCANCEL:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			case IDOK:
				demoWillStart = true;
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE; }
			break;

		case WM_CLOSE:
			wantFullScreen = SendMessage(GetDlgItem(hDlg, IDC_FULLSCREEN), BM_GETCHECK, 0, 0) > 0 ? true : false;
			want720p = SendMessage(GetDlgItem(hDlg, IDC_TDFMODE), BM_GETCHECK, 0, 0) > 0 ? true : false;
			//run_screenmode = SendMessage( GetDlgItem(hDlg,IDC_RES), CB_GETCURSEL, 0, 0 );
			DestroyWindow(hDlg);
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE; }
		return FALSE; }};


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE h0, LPSTR lpCmdLine, int nCmdShow) {
	using namespace rqdq::rqv;
	namespace jobsys = rqdq::rclmt::jobsys;
	namespace framepool = rqdq::rclma::framepool;

	SplashDialog splashDialog;
	splashDialog.create(hInst).show(nCmdShow).runUntilClosed();

	if (!splashDialog.demoWillStart) {
		return 0;}

	int threads = std::thread::hardware_concurrency();
	jobsys::init(threads);
	framepool::Init();
	auto app = Application();
	/*
	if (want720p) {
		//hackmode_enable = 1;
		//hackmode_width = 1280;
		//hackmode_height = 720; }
	else {
		hackmode_enable = 0; }
	*/
	app.SetNice(true);
	app.SetFullScreen(splashDialog.wantFullScreen);
	// app.startedFromGUI = true;  XXX for modesetting
	app.Run();
	return 0; }
