#pragma once
#include <string>

#include <Windows.h>

namespace rqdq {
namespace rclw {

struct CLSIDSerializer {
	static CLSID Deserialize(const std::wstring&);
};


}  // close package namespace
}  // close enterprise namespace
