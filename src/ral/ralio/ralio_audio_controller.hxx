#pragma once
#include <optional>
#include <string>

namespace rqdq {
namespace ralio {

class AudioStream {
	friend class AudioController;
	explicit AudioStream(unsigned long hStream);

public:
	AudioStream(const AudioStream&) = delete;
	AudioStream& operator=(const AudioStream& other) = delete;
	AudioStream(AudioStream&& other) noexcept;
	AudioStream& operator=(AudioStream&& other) noexcept;
	~AudioStream() noexcept;

	void Play() const;
	void Pause() const;
	bool IsPlaying() const;
	double GetPosition() const;
	void SetPosition(double seconds);

private:
	unsigned long hStream_{0}; };


class AudioController {
public:
	AudioController();
	std::optional<AudioStream> CreateStream(std::string /*path*/);
	void Start();
	void FillBuffers();
	~AudioController(); };


}  // namespace ralio
}  // namespace rqdq
