#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace rqdq {
namespace rqv {

#define APPCONFIG_ITEMS \
    X(configPath, std::string) \
	X(debug, bool) \
	X(telemetryScale, int) \
	X(nice, bool) \
	X(concurrency, int) \
	X(nodePath, std::vector<std::string>) \
	X(latencyInFrames, int) \
	X(textureDir, std::string) \
	X(meshDir, std::string) \
	X(fullScreen, bool) \
	X(outputSizeInPx, rmlv::ivec2) \
	X(tileSizeInBlocks, rmlv::ivec2)

struct PartialAppConfig {
#define X(n,t) std::optional<t> n{};
	APPCONFIG_ITEMS
#undef X
	};

struct AppConfig {
#define X(n,t) t n;
	APPCONFIG_ITEMS
#undef X
};

auto GetEnvConfig() -> PartialAppConfig;
auto GetFileConfig(const std::string& fn) -> PartialAppConfig;
auto GetArgConfig(int argc, char **argv) -> PartialAppConfig;

inline
auto Merge(PartialAppConfig a, const PartialAppConfig& b) -> PartialAppConfig {
#define X(n,t) if (b.n.has_value()) a.n = b.n.value();
	APPCONFIG_ITEMS
#undef X
	return a; }

inline
auto Solidify(const PartialAppConfig& a) -> AppConfig {
	AppConfig out;
#define X(n,t) assert(a.n.has_value()); out.n = a.n.value();
	APPCONFIG_ITEMS
#undef X
	return out; }

#undef APPCONFIG_ITEMS



class Application {
	class impl;

public:
	Application(const PartialAppConfig&);
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
