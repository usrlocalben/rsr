#pragma once
#include "src/ral/ralio/ralio_audio_controller.hxx"

#include <functional>
#include <memory>

namespace rqdq {
namespace rals {


class SyncController {
	struct impl;
	std::unique_ptr<impl> d_pImpl;

public:
	SyncController(std::string pathPrefix, ralio::AudioStream& soundtrack, double rowsPerSecond);
	~SyncController();
	SyncController(SyncController&&) = default;
	SyncController& operator=(SyncController&&) = default;
	
	void Connect();
	void AddTrack(const std::string&);
	double GetPositionInRows();

	void ForEachValue(double, std::function<void(const std::string&, double value)>);

	int Update(int positionInRows);
	void SaveTracks();
};

}  // close package namespace
}  // close enterprise namespace
