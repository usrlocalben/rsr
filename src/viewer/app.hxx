#pragma once
#include <memory>

namespace rqdq {
namespace rqv {

class Application {
	class impl;

public:
	Application();
	~Application();

	Application(Application&&) = default;
	Application(const Application&) = delete;
	Application& operator=(Application&& /*unused*/) noexcept;
	Application& operator=(const Application&) = delete;

	Application& SetNice(bool /*value*/);
	Application& SetFullScreen(bool /*value*/);
	Application& Run();

private:
	std::unique_ptr<impl> impl_; };


}  // namespace rqv
}  // namespace rqdq
