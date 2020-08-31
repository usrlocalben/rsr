#pragma once

#include "src/rgl/rglr/rglr_canvas.hxx"
#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

namespace rqdq {
namespace rglr {

struct QFloatFragmentCursor {

	using dataType = rmlv::qfloat;
	using elemType = rmlv::qfloat;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloatFragmentCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		data = _mm_load_ps(reinterpret_cast<float*>(&(buf_[offs_]))); }

	void Store(dataType destDepth, dataType sourceDepth, rmlv::mvec4i fragMask) {
		auto addr = &(buf_[offs_]);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }};


/**
 * qfloat3 in a qfloat4 buffer's rgb channels
 */
struct QFloat3FragmentCursor {

	using dataType = rmlv::qfloat3;
	using elemType = rmlv::qfloat3;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat3* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat3FragmentCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		rglr::QFloat3Canvas::Load(buf_+offs_, data.x.v, data.y.v, data.z.v); }

	void Store(dataType destColor, dataType sourceColor, rmlv::mvec4i fragMask) {
		auto sr = selectbits(destColor.r, sourceColor.r, fragMask).v;
		auto sg = selectbits(destColor.g, sourceColor.g, fragMask).v;
		auto sb = selectbits(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat3Canvas::Store(sr, sg, sb, buf_+offs_); } };

/**
 * qfloat3 in a qfloat4 buffer's rgb channels
 */
struct QFloat4RGBFragmentCursor {

	using dataType = rmlv::qfloat3;
	using elemType = rmlv::qfloat4;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat4* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat4RGBFragmentCursor(void* buf, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(buf)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		rglr::QFloat4Canvas::Load(buf_+offs_, data.x.v, data.y.v, data.z.v); }

	void Store(dataType destColor, dataType sourceColor, rmlv::mvec4i fragMask) {
		auto sr = selectbits(destColor.r, sourceColor.r, fragMask).v;
		auto sg = selectbits(destColor.g, sourceColor.g, fragMask).v;
		auto sb = selectbits(destColor.b, sourceColor.b, fragMask).v;
		rglr::QFloat4Canvas::Store(sr, sg, sb, buf_+offs_); } };


/**
 * qfloat in a qfloat4 buffer's alpha channel
 */
struct QFloat4AFragmentCursor {

	using dataType = rmlv::qfloat;
	using elemType = rmlv::qfloat4;
	static constexpr int strideInQuads = 64;
	static constexpr int tileAreaInQuads = 64*64;

	rmlv::qfloat4* buf_;
	const rmlv::ivec2 origin_;

	int offs_, offsLeft_;

	QFloat4AFragmentCursor(void* base, rmlv::ivec2 origin) :
		buf_(static_cast<elemType*>(base)),
		origin_(origin) {}

	void Begin(int x, int y) {
		x -= origin_.x;
		y -= origin_.y;
		offs_ = offsLeft_ = (y/2)*strideInQuads + (x/2); }

	void CR() {
		offsLeft_ += strideInQuads;
		offs_ = offsLeft_; }

	void Right2() {
		offs_++; }

	void Load(dataType& data) {
		data = _mm_load_ps(reinterpret_cast<float*>(&(buf_[offs_].a))); }

	void Store(dataType destDepth, dataType sourceDepth, rmlv::mvec4i fragMask) {
		auto addr = &(buf_[offs_].a);  // alpha-channel
		auto result = selectbits(destDepth, sourceDepth, fragMask).v;
		_mm_store_ps(reinterpret_cast<float*>(addr), result); }};


}  // close package namespace
}  // close enterprise namespace
