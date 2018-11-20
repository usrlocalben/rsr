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
	Application& operator=(Application&&);
	Application& operator=(const Application&) = delete;

	Application& setNice(bool);
	Application& setFullScreen(bool);
	Application& run();

private:
	std::unique_ptr<impl> d_pImpl;
};

}  // close package namespace
}  // close enterprise namespace
