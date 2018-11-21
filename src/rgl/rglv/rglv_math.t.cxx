#include <rglv_math.hxx>
#include <rmlm_mat4.hxx>
#include <rmlv_vec.hxx>

#include <string>
#include <iostream>

using namespace rqdq;
using namespace std;


int main(int argc, char **argv) {
	int total_tests{ 0 }, total_errors{ 0 };

	// test NDC coords to device coords
	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		rmlv::vec4 top_left{ -1.0, 1.0, 0, 1.0f };
		string test_name{ "top-left" };
		auto value = device_matrix * top_left;
		auto expected = rmlv::vec4{ 0,0,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}

	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		rmlv::vec4 bottom_left{ -1.0, -1.0, 0, 1.0f };
		string test_name{ "bottom-left" };
		auto value = device_matrix * bottom_left;
		auto expected = rmlv::vec4{ 0,240,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}

	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		rmlv::vec4 top_right{1.0, 1.0, 0, 1 };
		string test_name{ "top-right" };
		auto value = device_matrix * top_right;
		auto expected = rmlv::vec4{ 320,0,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}

	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		rmlv::vec4 bottom_right{ 1.0, -1.0, 0, 1.0f };
		string test_name{ "bottom-right" };
		auto value = device_matrix * bottom_right;
		auto expected = rmlv::vec4{ 320,240,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}


	// test ortho using a pixel-center projection
	// https://stackoverflow.com/questions/14608395/pixel-perfect-2d-rendering-with-opengl
	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		auto projection_matrix = rglv::make_glOrtho(-0.5, (320 - 1) + 0.5, (240 - 1) + 0.5, -0.5, -1.0, 1.0);
		rmlv::vec4 point{ 0.0, 0.0, 0, 1.0f };
		string test_name{ "orth top left" };
		auto value = projection_matrix * point;
		value = device_matrix * value;
		auto expected = rmlv::vec4{ 0.5,0.5,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}

	{
		total_tests++;
		auto device_matrix = rglv::make_device_matrix(320, 240);
		auto projection_matrix = rglv::make_glOrtho(-0.5, (320 - 1) + 0.5, (240 - 1) + 0.5, -0.5, -1.0, 1.0);
		rmlv::vec4 point{ 319.0, 239.0, 0, 1.0f };
		string test_name{ "ortho bottom right" };
		auto value = projection_matrix * point;
		value = device_matrix * value;
		auto expected = rmlv::vec4{ 319.5,239.5,0,1 };
		if (auto ok = rmlv::almost_equal(value, expected); !ok) {
			cout << test_name << ": expected " << expected << ", got " << value << "\n";
			total_errors++; }
		else {
			cout << "." << std::flush; }}

	cout << endl;
	return total_errors ? 1 : 0;
TEST(PerspectiveDivide, 
	vec4 pa{ 10,20,30,10 };
	assertAlmostEqual(perspective_divide(pa), vec4{ 1,2,3,1.0f / 10.0f });
}
