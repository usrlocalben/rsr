#include "src/viewer/node/fx_mc.hxx"

#include <memory>
#include <string>

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

using namespace std;


void FxMC::connect(const string& attr, NodeBase* other, const string& slot) {
	if (attr == "material") {
		material_node = dynamic_cast<MaterialNode*>(other); }
	else if (attr == "frob") {
		frob_node = dynamic_cast<ValuesBase*>(other);
		frob_slot = slot; }
	else {
		cout << "FxMC(" << name << ") attempted to add " << other->name << ":" << slot << " as " << attr << endl; } }


void FxMC::main() {
	using rmlv::vec3;
	namespace jobsys = rclmt::jobsys;

	swapBuffers();
	float frob_value = 0.0f;
	if (frob_node != nullptr) {
		frob_value = frob_node->get(frob_slot).as_float();
	}
	d_field.update(frob_value);

	AABB rootBlock = {
		vec3{-d_range, d_range,-d_range},
		vec3{ d_range,-d_range, d_range} };

	const int subDim = d_precision >> d_forkDepth;

	blockDivider.clear();
	blockDivider.compute(rootBlock, d_forkDepth);

	auto finalizeJob = finalize();
	std::array<jobsys::Job*, 4096> subJobs;  // enough for forkDepth==4
	int subJobEnd = 0;

	for (const auto& subBlock : blockDivider.results) {
		subJobs[subJobEnd++] = resolve(subBlock, subDim, finalizeJob); }
	for (int sji = 0; sji < subJobEnd; sji++) {
		jobsys::run(subJobs[sji]); }
	jobsys::run(finalizeJob); }


void FxMC::resolveImpl(const AABB block, const int dim, const int threadId) {
	using rmlv::vec3, rmlv::mvec4f, rmlv::qfloat, rmlv::qfloat3;
	const int stride = 64;
	std::array<std::array<float, stride*stride>, 2> buf;
	int top = 1;
	int bot = 0;

	float sy = block.left_top_back.y;
	float delta = (block.right_bottom_front.x - block.left_top_back.x) / float(dim);
	const qfloat vdelta{ delta * 4 };

	const auto mid = midpoint(block);
	const auto R = length(block.left_top_back - mid);
	float D = d_field.sample(mid);
	if (abs(D)*0.5f > R) {
		return; }

/*	auto fillSlice = [&](){
		float sz = block.left_top_back.z;
		for (int iz=0; iz<dim+1; iz++, sz += delta) {
			qfloat3 vpos{ block.left_top_back.x, sy, sz };
			vpos.x += mvec4f{ 0, delta, delta * 2, delta * 3 };
			for (int ix = 0; ix<dim+1; ix+=4, vpos += vdelta) {
				auto distance = d_field.sample(vpos);
				_mm_storeu_ps(&(buf[bot][iz*stride + ix]), distance.v); }}};*/

	auto fillSlice = [&](){
		float sz = block.left_top_back.z;
		for (int iz=0; iz<dim+1; iz++, sz += delta) {
			vec3 pos{ block.left_top_back.x, sy, sz };
			for (int ix = 0; ix<dim+1; ix+=1, pos.x += delta) {
				auto distance = d_field.sample(pos);
				buf[bot][iz*stride + ix] = distance; }}};

	fillSlice(); // load the first slice
	sy -= delta;
	auto& vao = allocVAO();

	vec3 origin = block.left_top_back;
	for (int iy=0; iy<dim; iy++, sy -= delta) {
		origin.y -= delta;
		std::swap(top, bot);
		fillSlice();

		origin.z = block.left_top_back.z;
		for (int iz = 0; iz < dim; iz++, origin.z += delta) {
			origin.x = block.left_top_back.x;
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
				rglv::march_sdf_vao(vao, origin, delta, cell, d_field); }}}}


void FxMC::draw(rglv::GL* _dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job* link, int depth) {
	auto& dc = *_dc;
	using rglv::GL_UNSIGNED_SHORT;
	using rglv::GL_CULL_FACE;
	using rglv::GL_PROJECTION;
	using rglv::GL_MODELVIEW;
	using rglv::GL_TRIANGLES;
	std::lock_guard<std::mutex> lock(dc.mutex);
	if (material_node != nullptr) {
		material_node->apply(_dc); }
	dc.glMatrixMode(GL_PROJECTION);
	dc.glLoadMatrix(*pmat);
	dc.glMatrixMode(GL_MODELVIEW);
	dc.glLoadMatrix(*mvmat);
	auto& buffer = d_buffers[d_activeBuffer];
	for (int ai=0; ai<d_bufferEnd[d_activeBuffer]; ai++) {
		auto& vao = buffer[ai];
		if (vao.size() != 0) {
			const int elements = vao.size();
			vao.pad();
			dc.glUseArray(vao);
			dc.glDrawArrays(GL_TRIANGLES, 0, elements);
		}}
	if (link != nullptr) {
		rclmt::jobsys::run(link); } }


void FxMC::swapBuffers() {
	d_activeBuffer++;
	if (d_activeBuffer == 3) {
		d_activeBuffer = 0; }
	d_bufferEnd[d_activeBuffer] = 0; }


rglv::VertexArray_F3F3F3& FxMC::allocVAO() {
	std::scoped_lock lock(d_bufferMutex);
	auto& buffer = d_buffers[d_activeBuffer];
	auto& end = d_bufferEnd[d_activeBuffer];
	if (end == buffer.size()) {
		if (buffer.capacity() == buffer.size()) {
			std::cout << "buffer is full @ " << buffer.size() << std::endl;
			throw std::runtime_error("buffer full"); }
		buffer.push_back({}); }
	auto& vao = buffer[end++];
	vao.clear();
	return vao; }


}  // namespace rqv
}  // namespace rqdq
