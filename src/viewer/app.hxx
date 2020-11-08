#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rqdq {
namespace rqv {

struct AppConfig {
	std::optional<std::string> configPath{};
	std::optional<bool> debug{};
	std::optional<int> telemetryScale{};
	std::optional<bool> nice{};
	std::optional<int> concurrency{};
	std::optional<std::vector<std::string>> nodePath{};
	std::optional<int> latencyInFrames{};
	std::optional<std::string> textureDir{};
	std::optional<std::string> meshDir{};
	std::optional<bool> fullScreen{};
	std::optional<rmlv::ivec2> outputSizeInPx{};
	std::optional<rmlv::ivec2> tileSizeInBlocks{}; };

#define MEMBERLIST \
    X(configPath) \
	X(debug) \
	X(telemetryScale) \
	X(nice) \
	X(concurrency) \
	X(nodePath) \
	X(latencyInFrames) \
	X(textureDir) \
	X(meshDir) \
	X(fullScreen) \
	X(outputSizeInPx) \
	X(tileSizeInBlocks)

auto GetEnvConfig() -> AppConfig;
auto GetFileConfig(const std::string& fn) -> AppConfig;
auto GetArgConfig(int argc, char **argv) -> AppConfig;

inline
auto Merge(AppConfig a, AppConfig b) -> AppConfig {
#define X(n) if (b.n.has_value()) a.n = b.n.value();
	MEMBERLIST
#undef X
	return a; }

#undef MEMBERLIST



class Application {
	class impl;

public:
	Application(AppConfig);
	~Application();

	Application(Application&&) = default;
	Application(const Application&) = delete;
	Application& operator=(Application&& /*unused*/) noexcept;
	Application& operator=(const Application&) = delete;
	Application& Run();

private:
	std::unique_ptr<impl> impl_; };


}  // namespace rqv
}  // namespace rqdq
