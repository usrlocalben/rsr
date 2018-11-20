#pragma once
#include <ralio_audio_controller.hxx>

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
	
	void connect();
	void addTrack(const std::string&);
	double positionInRows();

	void forEachValue(double, std::function<void(const std::string&, double value)>);

	int update(int positionInRows);
	void saveTracks();
};

}  // close package namespace
}  // close enterprise namespace
