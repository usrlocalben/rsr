#include "src/ral/ralio/ralio_audio_controller.hxx"

#include <cassert>
#include <optional>
#include <string>

#include <bass.h>


namespace rqdq {
namespace ralio {


AudioStream::~AudioStream() {
	if (d_hstream) {
		BASS_StreamFree(d_hstream); }}


void AudioStream::play() const {
	assert(d_hstream);
	BASS_ChannelPlay(d_hstream, false); }


void AudioStream::pause() const {
	assert(d_hstream);
	BASS_ChannelPause(d_hstream); }


bool AudioStream::isPlaying() const {
	assert(d_hstream);
	return BASS_ChannelIsActive(d_hstream) == BASS_ACTIVE_PLAYING; }


double AudioStream::position() const {
	assert(d_hstream);
	QWORD pos = BASS_ChannelGetPosition(d_hstream, BASS_POS_BYTE);
	double seconds = BASS_ChannelBytes2Seconds(d_hstream, pos);
	return seconds; }


void AudioStream::setPosition(double seconds) {
	assert(d_hstream);
	QWORD pos = BASS_ChannelSeconds2Bytes(d_hstream, seconds);
	BASS_ChannelSetPosition(d_hstream, pos, BASS_POS_BYTE); }


AudioController::AudioController() {
	auto success = BASS_Init(-1, 44100, 0, 0, 0);
	if (!success) {
		throw std::runtime_error("failed to initialize BASS system"); }}


std::optional<AudioStream> AudioController::createStream(std::string path) {
	auto stream = BASS_StreamCreateFile(false, path.c_str(), 0, 0, BASS_STREAM_PRESCAN);
	if (!stream) {
		return {}; }
	else {
		return AudioStream(stream); }}


void AudioController::start() {
	BASS_Start(); }


void AudioController::fillBuffers() {
	BASS_Update(0); }


AudioController::~AudioController() {
	BASS_Free(); }


}  // close package namespace
}  // close enterprise namespace
