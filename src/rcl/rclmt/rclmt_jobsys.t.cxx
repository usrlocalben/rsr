#include <rclmt_jobsys.hxx>
#include <rcls_timer.hxx>

#include <iostream>
#include <thread>
#include <vector>

using namespace rqdq;
namespace jobsys = rclmt::jobsys;


int main() {
	const auto hardware_threads = std::thread::hardware_concurrency();
	jobsys::init(hardware_threads);

	rcls::Timer tm;
	for (int frame = 0; frame < 6000; frame++) {
		jobsys::Job *root = jobsys::make_job(&jobsys::noop);
		for (int i = 0; i < 10000; i++) {
			jobsys::run(jobsys::make_job_as_child(root, &jobsys::noop)); }
		jobsys::run(root);
		jobsys::wait(root);
		jobsys::reset(); }
	std::cout << "6000 frames, " << tm.elapsed() << std::endl;

	jobsys::stop();
	jobsys::join();
	return 0; }
