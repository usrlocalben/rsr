#include "jobsys_vis.hxx"

#include <array>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglr/rglr_canvas.hxx"

namespace rqdq {
namespace {

auto tc_mul(const PixelToaster::TrueColorPixel& px, const float a) {
	auto mul = [](uint8_t a, float b) { return uint8_t(float(a) * b); };
	return PixelToaster::TrueColorPixel(
		mul(px.r, a),
		mul(px.g, a),
		mul(px.b, a)); }


}  // namespace

namespace rqv {

namespace jobsys = rclmt::jobsys;

void render_jobsys(const int left, const int top, const float xscale, rglr::TrueColorCanvas& canvas) {

	const int thick = 8;
	const int gap = 2;
	const auto scale = xscale * canvas.width();

	// some colors from I want hue, and a function to cycle over them
	// int color_idx = 0;
	constexpr int color_count = 16;
	constexpr int color_mask = color_count - 1;
	static const std::array<PixelToaster::TrueColorPixel, color_count> task_colors{ {
		PixelToaster::TrueColorPixel(0xd7de48), PixelToaster::TrueColorPixel(0xda7bf5),
		PixelToaster::TrueColorPixel(0x4ccbdb), PixelToaster::TrueColorPixel(0xf67e77),
		PixelToaster::TrueColorPixel(0x6dd671), PixelToaster::TrueColorPixel(0x84a6e6),
		PixelToaster::TrueColorPixel(0xee913c), PixelToaster::TrueColorPixel(0xf084b4),
		PixelToaster::TrueColorPixel(0x50d9a7), PixelToaster::TrueColorPixel(0x66de3e),
		PixelToaster::TrueColorPixel(0xd19bdf), PixelToaster::TrueColorPixel(0xcfa93c),
		PixelToaster::TrueColorPixel(0x9fc34f), PixelToaster::TrueColorPixel(0xf179d9),
		PixelToaster::TrueColorPixel(0xb4e532), PixelToaster::TrueColorPixel(0xebc630)
	} };

	// create raster data from a JobStat instance
	struct job_span {
		int left;
		int right;
		PixelToaster::TrueColorPixel color;
	};

	auto to_span = [=](const struct jobsys::JobStat& jobstat, const float scale) {
		const auto left = int(jobstat.start_time * scale);
		const auto right = int(jobstat.end_time * scale);

		// use some random(ish) bits to make a stable color selection
		auto randomish_bits = uint32_t(jobstat.raw);
		uint8_t ax = 0;
		ax ^= (randomish_bits & 0xff);
		randomish_bits >>= 6;
		ax ^= (randomish_bits & 0xff);
		randomish_bits >>= 8;
		ax ^= (randomish_bits & 0xff);
		randomish_bits >>= 10;
		ax ^= (randomish_bits & 0xff);

		const auto color = task_colors[ax & color_mask];
		//const auto color = task_colors[(randomish_bits >> 11) & color_mask];
		return job_span{ left, right, color }; };

	// render to canvas
	int bar_top = 0;
	for (const auto& thread_measurements : jobsys::measurements_pt) {
		for (const auto& jobstat : thread_measurements) {

			auto span = to_span(jobstat, scale);
			auto bright = 1.0F;
			auto delta = 1.0F / (span.right - span.left);

			for (int bx = span.left; bx <= span.right; bx++) {
				auto color = tc_mul(span.color, bright);

				for (int by = 0; by < thick; by++) {
					if (left + bx < canvas.width()) {
						canvas.data()[(top + bar_top + by) * canvas.stride() + left + bx] = color; }}
				bright -= delta; }}

		bar_top += thick + gap; }}


}  // namespace rqv
}  // namespace rqdq
