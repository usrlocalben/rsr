#include "rals_sync_controller.hxx"

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "src/ral/ralio/ralio_audio_controller.hxx"

#include "3rdparty/rocket/rocket/sync.h"

namespace rqdq {
namespace rals {

struct SyncController::impl {
	impl() = delete;
	impl(impl&& other) = delete;
	impl(const impl&) = delete;

	impl(std::string pathPrefix, ralio::AudioStream& audio, double precisionInRowsPerSecond)
		:audio_(audio), precisionInRowsPerSecond_(precisionInRowsPerSecond) {
		syncDevice_ = sync_create_device(pathPrefix.c_str());
		if (syncDevice_ == nullptr) {
			throw std::runtime_error("failed to create rocket"); }}

	~impl() {
		sync_destroy_device(syncDevice_); }

#ifndef SYNC_PLAYER
	void Connect(bool throwOnError=true) {
		auto error = sync_tcp_connect(syncDevice_, "localhost", SYNC_DEFAULT_PORT);
		connected_ = (error == 0);
		if (throwOnError && (error != 0)) {
			throw std::runtime_error("could not connect to rocket host and throwOnError=true"); }}
#endif

	const sync_track* GetTrack(const std::string& name) {
		return sync_get_track(syncDevice_, name.c_str()); }

	void AddTrack(std::string name) {
		auto search = tracks_.find(name);
		if (search == tracks_.end()) {
			auto t = sync_get_track(syncDevice_, name.c_str());
			tracks_[name] = t; }}

	template <typename FUNC>
	void ForEachValue(double musicPositionInRows, FUNC& func) {
		for (const auto& item : tracks_) {
			const auto& name = item.first;
			auto value = sync_get_val(item.second, musicPositionInRows);
			func(name, value); }}

#ifndef SYNC_PLAYER
	void SaveTracks() {
		sync_save_tracks(syncDevice_); }
#endif

	double GetPositionInRows() {
		double positionInSeconds = audio_.GetPosition();
		double positionInRows = positionInSeconds * precisionInRowsPerSecond_;
		return positionInRows; }

#ifndef SYNC_PLAYER
	static void cb_Pause(void* data, int flag) {
		impl& self = *reinterpret_cast<impl*>(data);
		if (flag != 0) {
			self.audio_.Pause(); }
		else {
			self.audio_.Play(); }}

	static void cb_SetPosition(void* data, int newPositionInRows) {
		impl& self = *reinterpret_cast<impl*>(data);
		auto newPositionInSeconds = newPositionInRows / self.precisionInRowsPerSecond_;
		self.audio_.SetPosition(newPositionInSeconds); }

	static int cb_IsPlaying(void* data) {
		impl& self = *reinterpret_cast<impl*>(data);
		return static_cast<int>(self.audio_.IsPlaying()); }

	const struct sync_cb syncCallbacks_ = { cb_Pause, cb_SetPosition, cb_IsPlaying };

	auto Update(int ipos) {
		return sync_update(syncDevice_, ipos, const_cast<struct sync_cb*>(&syncCallbacks_), (void*)this); }
#endif // SYNC_PLAYER

	ralio::AudioStream& audio_;
	sync_device* syncDevice_{nullptr};
	double precisionInRowsPerSecond_{0.0};
	std::unordered_map<std::string, const sync_track*> tracks_;
#ifndef SYNC_PLAYER
	bool connected_{false};
#endif
};

// SyncController::SyncController() = delete;
SyncController::~SyncController() = default;

SyncController::SyncController(std::string pathPrefix,
							   ralio::AudioStream& soundtrack,
							   double precisionInRowsPerSecond)
	:impl_(std::make_unique<impl>(pathPrefix, soundtrack, precisionInRowsPerSecond)) {}

#ifndef SYNC_PLAYER
void SyncController::Connect() {
	impl_->Connect(); }
int SyncController::Update(int positionInRows) {
	return impl_->Update(positionInRows); }
void SyncController::SaveTracks() {
	impl_->SaveTracks(); }
#endif
void SyncController::AddTrack(const std::string& name) {
	impl_->AddTrack(name); }
double SyncController::GetPositionInRows() {
	return impl_->GetPositionInRows(); }
void SyncController::ForEachValue(double positionInRows, std::function<void(const std::string&, double value)> func) {
	impl_->ForEachValue(positionInRows, func); }


}  // namespace rals
}  // namespace rqdq
