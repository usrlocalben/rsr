#include <rclt_util.hxx>

#include <iostream>
#include <string>

using namespace rqdq;


void ok() {
	std::cout << "."; }

void fail() {
	std::cout << "F"; }


void assertEqual(const std::string& a, const std::string& b) {
	if (a == b) {
		ok();
		return; }
	std::cout << "error, strings not equal " << a << " != " << b << std::endl; }


void test_trim() {
	assertEqual(rclt::trim("   hey   "), "hey");
	assertEqual(rclt::trim("   hey"), "hey");
	assertEqual(rclt::trim("hey   "), "hey");
	assertEqual(rclt::trim("hey"), "hey"); }


int main(int argc, char **argv) {
	test_trim();
	return 0; }
