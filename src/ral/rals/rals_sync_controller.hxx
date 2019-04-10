#pragma once
#include <functional>
#include <memory>

#include "src/ral/ralio/ralio_audio_controller.hxx"

namespace rqdq {
namespace rals {

class SyncController {
	struct impl;
	std::unique_ptr<impl> impl_;

public:
	SyncController(std::string pathPrefix, ralio::AudioStream& soundtrack, double rowsPerSecond);
	~SyncController();
	SyncController(SyncController&&) = default;
	SyncController& operator=(SyncController&&) = default;
	
	void Connect();
	void AddTrack(const std::string& /*name*/);
	double GetPositionInRows();

	void ForEachValue(double /*positionInRows*/, std::function<void(const std::string&, double value)> /*func*/);

	int Update(int positionInRows);
	void SaveTracks(); };


}  // namespace rals
}  // namespace rqdq
