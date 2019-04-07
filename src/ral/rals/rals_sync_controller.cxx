#include "src/ral/rals/rals_sync_controller.hxx"
#include "src/ral/ralio/ralio_audio_controller.hxx"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "3rdparty/rocket/rocket/sync.h"


namespace rqdq {
namespace rals {

struct SyncController::impl {
	impl() = delete;
	impl(impl&& other) = delete;
	impl(const impl&) = delete;

	impl(std::string pathPrefix, ralio::AudioStream& audio, double rowsPerSecond)
		:d_audio(audio), d_rowsPerSecond(rowsPerSecond) {
		d_syncDevice = sync_create_device(pathPrefix.c_str());
		if (d_syncDevice == nullptr) {
			throw std::runtime_error("failed to create rocket"); }}

	~impl() {
		sync_destroy_device(d_syncDevice); }

#ifndef SYNC_PLAYER
	void Connect(bool throwOnError=true) {
		auto error = sync_tcp_connect(d_syncDevice, "localhost", SYNC_DEFAULT_PORT);
		d_connected = (error == 0);
		if (throwOnError && (error != 0)) {
			throw std::runtime_error("could not connect to rocket host and throwOnError=true"); }}
#endif

	const sync_track* GetTrack(const std::string& name) {
		return sync_get_track(d_syncDevice, name.c_str()); }

	void AddTrack(std::string name) {
		auto search = d_tracks.find(name);
		if (search == d_tracks.end()) {
			auto t = sync_get_track(d_syncDevice, name.c_str());
			d_tracks[name] = t; }}

	template <typename FUNC>
	void ForEachValue(double musicPositionInRows, FUNC& func) {
		for (const auto& item : d_tracks) {
			const auto& name = item.first;
			auto value = sync_get_val(item.second, musicPositionInRows);
			func(name, value); }}

#ifndef SYNC_PLAYER
	void SaveTracks() {
		sync_save_tracks(d_syncDevice); }
#endif

	double GetPositionInRows() {
		double positionInSeconds = d_audio.GetPosition();
		double positionInRows = positionInSeconds * d_rowsPerSecond;
		return positionInRows; }

#ifndef SYNC_PLAYER
	static void cb_Pause(void* data, int flag) {
		impl& self = *reinterpret_cast<impl*>(data);
		if (flag != 0) {
			self.d_audio.Pause(); }
		else {
			self.d_audio.Play(); }}

	static void cb_SetPosition(void* data, int newPositionInRows) {
		impl& self = *reinterpret_cast<impl*>(data);
		auto newPositionInSeconds = newPositionInRows / self.d_rowsPerSecond;
		self.d_audio.SetPosition(newPositionInSeconds); }

	static int cb_IsPlaying(void* data) {
		impl& self = *reinterpret_cast<impl*>(data);
		return static_cast<int>(self.d_audio.IsPlaying()); }

	const struct sync_cb d_syncCallbacks = { cb_Pause, cb_SetPosition, cb_IsPlaying };

	auto Update(int ipos) {
		return sync_update(d_syncDevice, ipos, const_cast<struct sync_cb*>(&d_syncCallbacks), (void*)this); }
#endif // SYNC_PLAYER

	sync_device* d_syncDevice = nullptr;
	double d_rowsPerSecond = 0;
	ralio::AudioStream& d_audio;
	std::unordered_map<std::string, const sync_track*> d_tracks;
#ifndef SYNC_PLAYER
	bool d_connected = false;
#endif
};

// SyncController::SyncController() = delete;
SyncController::~SyncController() = default;

SyncController::SyncController(std::string pathPrefix,
							   ralio::AudioStream& soundtrack,
							   double rowsPerSecond)
	:d_pImpl(std::make_unique<impl>(pathPrefix, soundtrack, rowsPerSecond)) {}

#ifndef SYNC_PLAYER
void SyncController::Connect() { d_pImpl->Connect(); }
int SyncController::Update(int positionInRows) { return d_pImpl->Update(positionInRows); }
void SyncController::SaveTracks() { d_pImpl->SaveTracks(); }
#endif
void SyncController::AddTrack(const std::string& name) { d_pImpl->AddTrack(name); }
double SyncController::GetPositionInRows() { return d_pImpl->GetPositionInRows(); }
void SyncController::ForEachValue(double positionInRows, std::function<void(const std::string&, double value)> func) { d_pImpl->ForEachValue(positionInRows, func); }


}  // namespace rals
}  // namespace rqdq
