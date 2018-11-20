#include <rmlv_vec.hxx>
#include <rmlm_mat4.hxx>

#include <string>
#include <iostream>

using namespace rqdq;
using namespace std;


void ok() {
	cout << "."; }


void fail() {
	cout << "F"; }


void assertAlmostEqual(const rmlv::vec4& a, const rmlv::vec4& b) {
	static const auto ep = 0.001f;
	float dx = abs(a.x - b.x);
	float dy = abs(a.y - b.y);
	float dz = abs(a.z - b.z);
	float dw = abs(a.w - b.w);
	if (dx < ep && dy < ep && dz < ep && dw < ep) {
		ok();
		return; }
	cout << "error, not almost equal " << a << ", !~= " << b << endl; }


void test_mat4_basic() {
	rmlm::mat4 m = rmlm::mat4::ident();

	rmlv::vec4 a{ 3, 4, 5, 6 };
	assertAlmostEqual(m*a, rmlv::vec4{ 3,4,5,6 });

	rmlv::vec4 b{ 10, 10, 10, 1 };
	m = rmlm::mat4::scale(rmlv::vec4{ 1, 2, 3, 4 });  // w component should be dropped
	assertAlmostEqual(m*b, rmlv::vec4{ 10, 20, 30, 1 });

	a = rmlv::vec4{ 3, 4, 5, 1 };
	m = rmlm::mat4::translate(0, 0, -10);
	assertAlmostEqual(m*a, rmlv::vec4{ 3, 4, -5, 1 });

	m = rmlm::mat4{
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15 };
	assertAlmostEqual(m.ff[0], 0);
	assertAlmostEqual(m.ff[1], 4);
	assertAlmostEqual(m.ff[2], 8);
	assertAlmostEqual(m.ff[3], 12);

	assertAlmostEqual(m.ff[4], 1);
	assertAlmostEqual(m.ff[5], 5);
	assertAlmostEqual(m.ff[6], 9);
	assertAlmostEqual(m.ff[7], 13);

	// skip the third col...

	assertAlmostEqual(m.ff[12], 3);
	assertAlmostEqual(m.ff[13], 7);
	assertAlmostEqual(m.ff[14], 11);
	assertAlmostEqual(m.ff[15], 15);
}


int main(int argc, char **argv) {
	test_mat4_basic();
}
