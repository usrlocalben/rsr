#pragma once
#include "src/rml/rmlv/rmlv_vec.hxx"
// #include "src/viewer/node/base.hxx"

namespace rqdq {
namespace rqv {

/**
 * controller that takes input before a frame
 * e.g. game script
 */
class IController {
public:
	virtual void KeyPress(int) = 0;
	virtual void BeforeFrame(float) = 0; };


}  // namespace rqv
}  // namespace rqdq
