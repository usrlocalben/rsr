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

	void Play() const;
	void Pause() const;
	bool IsPlaying() const;
	double GetPosition() const;
	void SetPosition(double seconds);

private:
	unsigned long d_hstream = 0; };


class AudioController {
public:
	AudioController();
	std::optional<AudioStream> CreateStream(std::string);
	void Start();
	void FillBuffers();
	~AudioController(); };


}  // close package namespace
}  // close enterprise namespace
