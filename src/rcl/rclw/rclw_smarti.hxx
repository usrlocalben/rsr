#pragma once

namespace rqdq {
namespace rclw {

/**
 * smart pointer for COM objects
 * sourced from PixelToasterWindows.h
 */
template <typename I>
class SmartI {
public:
	SmartI(I* i=nullptr): i_(i) {}
	SmartI(SmartI& other): i_(other.i_) { if (i_) i_->AddRef(); }
	~SmartI() { if (i_) i_->Release(); i_ = nullptr; }
	SmartI& operator=(SmartI& other) { SmartI temp(other); swap(temp); return *this; }

	void reset(I* i = nullptr) { SmartI temp(i); swap(temp); }
	I* const get() const { return i_; }
	I* const operator->() const { return get(); }
	I** address() { return &i_; }
	void swap(SmartI& other) { I* i = i_; i_ = other.i_; other.i_ = i; }
	const bool operator!() const { return i_ == nullptr; }

private:
	I* i_; };


}  // close package namespace
}  // close enterprise namespace
