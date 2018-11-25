#include <rals_sync_controller.hxx>
#include <ralio_audio_controller.hxx>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <rocket/sync.h>


namespace rqdq {
namespace rals {

struct SyncController::impl {
	impl() = delete;
	impl(impl&& other) = delete;
	impl(const impl&) = delete;

	impl(std::string pathPrefix, ralio::AudioStream& audio, double rowsPerSecond)
		:d_audio(audio), d_rowsPerSecond(rowsPerSecond) {
		d_syncDevice = sync_create_device(pathPrefix.c_str());
		if (!d_syncDevice) {
			throw std::runtime_error("failed to create rocket"); }}

	~impl() {
		sync_destroy_device(d_syncDevice); }

#ifndef SYNC_PLAYER
	void connect(bool throwOnError=true) {
		auto error = sync_tcp_connect(d_syncDevice, "localhost", SYNC_DEFAULT_PORT);
		d_connected = !error;
		if (throwOnError && error) {
			throw std::runtime_error("could not connect to rocket host and throwOnError=true"); }}
#endif

	const sync_track* getTrack(const std::string& name) {
		return sync_get_track(d_syncDevice, name.c_str()); }

	void addTrack(std::string name) {
		auto search = d_tracks.find(name);
		if (search == d_tracks.end()) {
			auto t = sync_get_track(d_syncDevice, name.c_str());
			d_tracks[name] = t; }}

	template <typename FUNC>
	void forEachValue(double musicPositionInRows, FUNC& func) {
		for (const auto& item : d_tracks) {
			const auto& name = item.first;
			auto value = sync_get_val(item.second, musicPositionInRows);
			func(name, value); }}

#ifndef SYNC_PLAYER
	void saveTracks() {
		sync_save_tracks(d_syncDevice); }
#endif

	double positionInRows() {
		double positionInSeconds = d_audio.position();
		double positionInRows = positionInSeconds * d_rowsPerSecond;
		return positionInRows; }

#ifndef SYNC_PLAYER
	static void cb_pause(void* data, int flag) {
		impl& self = *reinterpret_cast<impl*>(data);
		if (flag) {
			self.d_audio.pause(); }
		else {
			self.d_audio.play(); }}

	static void cb_setPosition(void* data, int newPositionInRows) {
		impl& self = *reinterpret_cast<impl*>(data);
		auto newPositionInSeconds = newPositionInRows / self.d_rowsPerSecond;
		self.d_audio.setPosition(newPositionInSeconds); }

	static int cb_isPlaying(void* data) {
		impl& self = *reinterpret_cast<impl*>(data);
		return self.d_audio.isPlaying(); }

	const struct sync_cb d_syncCallbacks = { cb_pause, cb_setPosition, cb_isPlaying };

	auto update(int ipos) {
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
void SyncController::connect() { d_pImpl->connect(); }
int SyncController::update(int positionInRows) { return d_pImpl->update(positionInRows); }
void SyncController::saveTracks() { d_pImpl->saveTracks(); }
#endif
void SyncController::addTrack(const std::string& name) { d_pImpl->addTrack(name); }
double SyncController::positionInRows() { return d_pImpl->positionInRows(); }
void SyncController::forEachValue(double positionInRows, std::function<void(const std::string&, double value)> func) { d_pImpl->forEachValue(positionInRows, func); }



}  // close package namespace
}  // close enterprise namespace
