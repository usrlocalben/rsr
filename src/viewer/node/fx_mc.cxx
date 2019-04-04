#include "fx_mc.hxx"

#include <iostream>
#include <memory>
#include <mutex>
#include <string_view>

#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rgl/rglv/rglv_gl.hxx"
//#include "src/rgl/rglv/rglv_mesh.hxx"
#include "src/rgl/rglv/rglv_vao.hxx"
#include "src/viewer/node/base.hxx"
#include "src/viewer/node/material.hxx"
#include "src/viewer/node/value.hxx"
//#include "src/viewer/shaders.hxx"

namespace rqdq {
namespace rqv {

void FxMC::Connect(std::string_view attr, NodeBase* other, std::string_view slot) {
	if (attr == "material") {
		materialNode_ = static_cast<MaterialNode*>(other); }
	else if (attr == "frob") {
		frobNode_ = static_cast<ValuesBase*>(other);
		frobSlot_ = slot; }
	else {
		GlNode::Connect(attr, other, slot); }}


void FxMC::Main() {
	using rmlv::vec3;
	namespace jobsys = rclmt::jobsys;

	SwapBuffers();

	float frobValue = 0.0f;
	if (frobNode_ != nullptr) {
		frobValue = frobNode_->Get(frobSlot_).as_float(); }
	field_.Update(frobValue);

	AABB rootBlock = {
		vec3{-range_, range_,-range_},
		vec3{ range_,-range_, range_} };

	const int subDim = precision_ >> forkDepth_;

	blockDivider_.Clear();
	blockDivider_.Compute(rootBlock, forkDepth_);

	auto finalizeJob = Finalize();
	std::array<jobsys::Job*, 4096> subJobs;  // enough for forkDepth==4
	int subJobEnd = 0;

	for (const auto& subBlock : blockDivider_.results_) {
		subJobs[subJobEnd++] = Resolve(subBlock, subDim, finalizeJob); }
	for (int sji = 0; sji < subJobEnd; sji++) {
		jobsys::run(subJobs[sji]); }
	jobsys::run(finalizeJob); }


void FxMC::ResolveImpl(const AABB block, const int dim, const int threadId) {
	using rmlv::vec3, rmlv::mvec4f, rmlv::qfloat, rmlv::qfloat3;
	const int stride = 64;
	std::array<std::array<float, stride*stride>, 2> buf;
	int top = 1;
	int bot = 0;

	float sy = block.leftTopBack.y;
	float delta = (block.rightBottomFront.x - block.leftTopBack.x) / float(dim);
	const qfloat vdelta{ delta * 4 };

	const auto mid = midpoint(block);
	const auto R = length(block.leftTopBack - mid);
	float D = field_.sample(mid);
	if (abs(D)*0.5f > R) {
		return; }

/*	auto fillSlice = [&](){
		float sz = block.leftTopBack.z;
		for (int iz=0; iz<dim+1; iz++, sz += delta) {
			qfloat3 vpos{ block.leftTopBack.x, sy, sz };
			vpos.x += mvec4f{ 0, delta, delta * 2, delta * 3 };
			for (int ix = 0; ix<dim+1; ix+=4, vpos += vdelta) {
				auto distance = field_.sample(vpos);
				_mm_storeu_ps(&(buf[bot][iz*stride + ix]), distance.v); }}};*/

	auto fillSlice = [&](){
		float sz = block.leftTopBack.z;
		for (int iz=0; iz<dim+1; iz++, sz += delta) {
			vec3 pos{ block.leftTopBack.x, sy, sz };
			for (int ix = 0; ix<dim+1; ix+=1, pos.x += delta) {
				auto distance = field_.sample(pos);
				buf[bot][iz*stride + ix] = distance; }}};

	fillSlice(); // load the first slice
	sy -= delta;
	auto& vao = AllocVAO();

	vec3 origin = block.leftTopBack;
	for (int iy=0; iy<dim; iy++, sy -= delta) {
		origin.y -= delta;
		std::swap(top, bot);
		fillSlice();

		origin.z = block.leftTopBack.z;
		for (int iz = 0; iz < dim; iz++, origin.z += delta) {
			origin.x = block.leftTopBack.x;
			for (int ix = 0; ix < dim; ix++, origin.x += delta) {
				rglv::Cell cell;
				cell.value[0] = buf[bot][ iz   *stride + ix];
				cell.pos[0] = origin;

				cell.value[1] = buf[bot][ iz   *stride + ix + 1];
				cell.pos[1] = vec3{ origin.x + delta, origin.y, origin.z };

				cell.value[2] = buf[top][ iz   *stride + ix + 1];
				cell.pos[2] = vec3{ origin.x + delta, origin.y + delta, origin.z };

				cell.value[3] = buf[top][ iz   *stride + ix];
				cell.pos[3] = vec3{ origin.x,         origin.y + delta, origin.z };

				cell.value[4] = buf[bot][(iz+1)*stride + ix];
				cell.pos[4] = vec3{ origin.x,         origin.y, origin.z + delta };

				cell.value[5] = buf[bot][(iz+1)*stride + ix + 1];
				cell.pos[5] = vec3{ origin.x + delta, origin.y, origin.z + delta };

				cell.value[6] = buf[top][(iz+1)*stride + ix + 1];
				cell.pos[6] = vec3{ origin.x + delta, origin.y + delta, origin.z + delta };

				cell.value[7] = buf[top][(iz+1)*stride + ix];
				cell.pos[7] = vec3{ origin.x,         origin.y + delta, origin.z + delta };
				rglv::march_sdf_vao(vao, origin, delta, cell, field_); }}}}


void FxMC::Draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	auto& dc = *_dc;
	using rglv::GL_UNSIGNED_SHORT;
	using rglv::GL_CULL_FACE;
	using rglv::GL_PROJECTION;
	using rglv::GL_MODELVIEW;
	using rglv::GL_TRIANGLES;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (materialNode_ != nullptr) {
		materialNode_->Apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	auto& buffer = buffers_[activeBuffer_];
	for (int ai=0; ai<bufferEnd_[activeBuffer_]; ai++) {
		auto& vao = buffer[ai];
		if (vao.size() != 0) {
			const int elements = vao.size();
			vao.pad();
			dc.glUseArray(vao);
			dc.glDrawArrays(GL_TRIANGLES, 0, elements); }}
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


void FxMC::SwapBuffers() {
	activeBuffer_ = (activeBuffer_+1) % 3;
	bufferEnd_[activeBuffer_] = 0; }


rglv::VertexArray_F3F3F3& FxMC::AllocVAO() {
	std::scoped_lock lock(bufferMutex_);
	auto& buffer = buffers_[activeBuffer_];
	auto& end = bufferEnd_[activeBuffer_];
	if (end == buffer.size()) {
		if (buffer.capacity() == buffer.size()) {
			std::cerr << "buffer is full @ " << buffer.size() << std::endl;
			throw std::runtime_error("buffer full"); }
		buffer.push_back({}); }
	auto& vao = buffer[end++];
	vao.clear();
	return vao; }


}  // namespace rqv
}  // namespace rqdq
