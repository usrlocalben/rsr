#pragma once

#include <optional>
#include <string>

namespace rqdq {
namespace ralio {

class AudioStream {
	friend class AudioController;
	AudioStream(unsigned long hstream) :d_hstream(hstream) {}

	AudioStream(const AudioStream&) = delete;
	AudioStream& operator=(const AudioStream& other) = delete;

public:
	AudioStream(AudioStream&& other) :d_hstream(other.d_hstream) { other.d_hstream = 0; }
	AudioStream& operator=(AudioStream&& other) {
		if (this != &other) {
			d_hstream = other.d_hstream;
			other.d_hstream = 0; }
		return *this; }

	~AudioStream();

	void play() const;
	void pause() const;
	bool isPlaying() const;
	double position() const;
	void setPosition(double seconds);

private:
	unsigned long d_hstream = 0; };


class AudioController {
public:
	AudioController();
	std::optional<AudioStream> createStream(std::string);
	void start();
	void fillBuffers();
	~AudioController(); };


}  // close package namespace
}  // close enterprise namespace
