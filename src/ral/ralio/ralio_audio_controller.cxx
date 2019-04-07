#include "src/ral/ralio/ralio_audio_controller.hxx"

#include <cassert>
#include <optional>
#include <string>

#include "3rdparty/bass/bass.h"


namespace rqdq {
namespace ralio {


AudioStream::~AudioStream() {
	if (d_hstream != 0u) {
		BASS_StreamFree(d_hstream); }}


void AudioStream::Play() const {
	assert(d_hstream);
	BASS_ChannelPlay(d_hstream, 0); }


void AudioStream::Pause() const {
	assert(d_hstream);
	BASS_ChannelPause(d_hstream); }


bool AudioStream::IsPlaying() const {
	assert(d_hstream);
	return BASS_ChannelIsActive(d_hstream) == BASS_ACTIVE_PLAYING; }


double AudioStream::GetPosition() const {
	assert(d_hstream);
	QWORD pos = BASS_ChannelGetPosition(d_hstream, BASS_POS_BYTE);
	double seconds = BASS_ChannelBytes2Seconds(d_hstream, pos);
	return seconds; }


void AudioStream::SetPosition(double seconds) {
	assert(d_hstream);
	QWORD pos = BASS_ChannelSeconds2Bytes(d_hstream, seconds);
	BASS_ChannelSetPosition(d_hstream, pos, BASS_POS_BYTE); }


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
