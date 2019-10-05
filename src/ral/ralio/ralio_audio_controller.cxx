#include "ralio_audio_controller.hxx"

#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>

#include "3rdparty/bass/bass.h"


namespace rqdq {
namespace ralio {

AudioStream::AudioStream(unsigned long hStream)
	:hStream_(hStream) {}


AudioStream::AudioStream(AudioStream&& other) noexcept
	:hStream_(other.hStream_) { other.hStream_ = 0; }


AudioStream& AudioStream::operator=(AudioStream&& other) noexcept {
	if (this != &other) {
		hStream_ = other.hStream_;
		other.hStream_ = 0; }
	return *this; }


AudioStream::~AudioStream() noexcept {
	if (hStream_ != 0u) {
		BASS_StreamFree(hStream_); }}


void AudioStream::Play() const {
	assert(hStream_);
	BASS_ChannelPlay(hStream_, 0); }


void AudioStream::Pause() const {
	assert(hStream_);
	BASS_ChannelPause(hStream_); }


bool AudioStream::IsPlaying() const {
	assert(hStream_);
	return BASS_ChannelIsActive(hStream_) == BASS_ACTIVE_PLAYING; }


double AudioStream::GetPosition() const {
	assert(hStream_);
	QWORD pos = BASS_ChannelGetPosition(hStream_, BASS_POS_BYTE);
	double seconds = BASS_ChannelBytes2Seconds(hStream_, pos);
	return seconds; }


void AudioStream::SetPosition(double seconds) {
	assert(hStream_);
	QWORD pos = BASS_ChannelSeconds2Bytes(hStream_, seconds);
	BASS_ChannelSetPosition(hStream_, pos, BASS_POS_BYTE); }


AudioController::AudioController() {
	auto success = BASS_Init(-1, 44100, 0, nullptr, nullptr);
	if (success == 0) {
		throw std::runtime_error("failed to initialize BASS system"); }}


std::optional<AudioStream> AudioController::CreateStream(std::string path) {
	auto stream = BASS_StreamCreateFile(0, path.c_str(), 0, 0, BASS_STREAM_PRESCAN);
	if (stream == 0u) {
		return {}; }
	return AudioStream(stream); }


void AudioController::Start() {
	BASS_Start(); }


void AudioController::FillBuffers() {
	BASS_Update(0); }


AudioController::~AudioController() {
	BASS_Free(); }


}  // namespace ralio
}  // namespace rqdq
