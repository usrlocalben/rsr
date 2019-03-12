#include "src/rcl/rclmt/rclmt_jobsys.hxx"
#include "src/rcl/rcls/rcls_timer.hxx"

#include <iostream>
#include <thread>
#include <vector>

using namespace rqdq;
namespace jobsys = rclmt::jobsys;

const int batches = 500;
const int jobsPerBatch = 10000;

int main() {
	const auto hardware_threads = std::thread::hardware_concurrency();
	jobsys::init(hardware_threads);

	rcls::Timer tm;
	for (int frame = 0; frame < batches; frame++) {
		jobsys::Job *root = jobsys::make_job(&jobsys::noop);
		for (int i = 0; i < jobsPerBatch; i++) {
			jobsys::run(jobsys::make_job_as_child(root, &jobsys::noop)); }
		jobsys::run(root);
		jobsys::wait(root);
		jobsys::reset(); }
	std::cout << batches << " batches of " << jobsPerBatch << " jobs, ";
	std::cout << "took " << tm.GetElapsed() << " seconds\n";

	jobsys::stop();
	jobsys::join();
	return 0; }
