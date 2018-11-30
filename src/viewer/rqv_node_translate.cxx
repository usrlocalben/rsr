#include "src/viewer/rqv_node_translate.hxx"

namespace rqdq {
namespace rqv {


void RepeatOp::connect(const std::string& attr, NodeBase* other, const std::string& slot) {
	if (attr == "gl") {
		lower = dynamic_cast<GlNode*>(other);
		return; }
	if (attr == "scale") {
		scale_source_node = static_cast<ValuesBase*>(other);
		scale_source_slot = slot;
		return; }
	if (attr == "rotate") {
		rotate_source_node = static_cast<ValuesBase*>(other);
		rotate_source_slot = slot;
		return; }
	if (attr == "translate") {
		translate_source_node = static_cast<ValuesBase*>(other);
		translate_source_slot = slot;
		return; }
	GlNode::connect(attr, other, slot); }


std::vector<NodeBase*> RepeatOp::deps() {
	std::vector<NodeBase*> out;
	out.push_back(lower);
	return out; }


void RepeatOp::main() {
	namespace jobsys = rclmt::jobsys;
	auto* my_noop = jobsys::make_job(jobsys::noop);
	add_links_to(my_noop);
	lower->add_link(my_noop);
	lower->run(); }


void RepeatOp::draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	using rmlm::mat4;
	using rmlv::M_PI;
	namespace jobsys = rclmt::jobsys;
	namespace framepool = rclma::framepool;

	const rmlv::vec3 translate = [&]() {
		if (translate_source_node != nullptr) {
			return translate_source_node->get(translate_source_slot).as_vec3(); }
		return translate_fixed;
		}();
	const rmlv::vec3 rotate = [&]() {
		if (rotate_source_node != nullptr) {
			return rotate_source_node->get(rotate_source_slot).as_vec3(); }
		return rotate_fixed;
		}();
	const rmlv::vec3 scale = [&]() {
		if (scale_source_node != nullptr) {
			return scale_source_node->get(scale_source_slot).as_vec3(); }
		return scale_fixed;
		}();

	if (cnt == 0) {
		rclmt::jobsys::run(link);
		return; }

	rmlv::vec3 p{0,0,0};
	rmlv::vec3 r{0,0,0};
	rmlv::vec3 s{1,1,1};
	auto& counter = *reinterpret_cast<std::atomic<int>*>(rclma::framepool::allocate(sizeof(std::atomic<int>)));
	counter = cnt;
	for (int i = 0; i < cnt; i++) {
		if (depth < DEPTH_FORK_UNTIL) {
			mat4& M = *reinterpret_cast<mat4*>(framepool::allocate(64));
			M = *mvmat  * mat4::rotate(r.x * M_PI * 2, 1, 0, 0);
			M = M * mat4::rotate(r.y * M_PI * 2, 0, 1, 0);
			M = M * mat4::rotate(r.z * M_PI * 2, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);
			jobsys::Job* then = jobsys::make_job(after_draw, std::tuple{&counter, link});
			jobsys::run(drawLower(dc, pmat, &M, then, depth+1)); }
		else {
			mat4 M;
			M = *mvmat * mat4::rotate(r.x * M_PI * 2, 1, 0, 0);
			M = M * mat4::rotate(r.y * M_PI * 2, 0, 1, 0);
			M = M * mat4::rotate(r.z * M_PI * 2, 0, 0, 1);
			M = M * mat4::translate(p);
			M = M * mat4::scale(s);
			lower->draw(dc, pmat, &M, nullptr, depth + 1); }
		p += translate;
		r += rotate;
		s *= scale; }
	if (depth >= DEPTH_FORK_UNTIL) {
		jobsys::run(link); } }


void TranslateOp::connect(const std::string & attr, NodeBase * other, const std::string & slot) {
	if (attr == "gl") {
		lower = dynamic_cast<GlNode*>(other);
		return; }
	if (attr == "scale") {
		scale_source_node = static_cast<ValuesBase*>(other);
		scale_source_slot = slot;
		return; }
	if (attr == "rotate") {
		rotate_source_node = static_cast<ValuesBase*>(other);
		rotate_source_slot = slot;
		return; }
	if (attr == "translate") {
		translate_source_node = static_cast<ValuesBase*>(other);
		translate_source_slot = slot;
		return; }
	GlNode::connect(attr, other, slot); }


std::vector<NodeBase*> TranslateOp::deps() {
	std::vector<NodeBase*> out;
	out.push_back(lower);
	return out; }


void TranslateOp::main() {
	auto* my_noop = rclmt::jobsys::make_job(rclmt::jobsys::noop);
	add_links_to(my_noop);
	lower->add_link(my_noop);
	lower->run(); }


void TranslateOp::draw(rglv::GL* dc, const rmlm::mat4* const pmat, const rmlm::mat4* const mvmat, rclmt::jobsys::Job *link, int depth) {
	using rmlm::mat4;
	using rmlv::M_PI;
	namespace framepool = rclma::framepool;
	namespace jobsys = rclmt::jobsys;

	mat4& M = *reinterpret_cast<mat4*>(framepool::allocate(64));

	rmlv::vec3 scale = {1.0f, 1.0f, 1.0f};
	if (scale_source_node != nullptr) {
		scale = scale_source_node->get(scale_source_slot).as_vec3(); }

	rmlv::vec3 rotate = {0,0,0};
	if (rotate_source_node != nullptr) {
		rotate = rotate_source_node->get(rotate_source_slot).as_vec3(); }

	rmlv::vec3 translate = {0,0,0};
	if (translate_source_node != nullptr) {
		translate = translate_source_node->get(translate_source_slot).as_vec3(); }

	M = *mvmat * mat4::scale(scale);
	M = M * mat4::rotate(rotate.x * M_PI * 2, 1, 0, 0);
	M = M * mat4::rotate(rotate.y * M_PI * 2, 0, 1, 0);
	M = M * mat4::rotate(rotate.z * M_PI * 2, 0, 0, 1);
	M = M * mat4::translate(translate);
	lower->draw(dc, pmat, &M, link, depth); }


}  // close package namespace
}  // close enterprise namespace
