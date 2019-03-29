#include "src/rml/rmlv/rmlv_vec.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"

#include <string>
#include <iostream>
#include <gtest/gtest.h>

using namespace rqdq;
using namespace std;

namespace {

#define EXPECT_VEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x, val2.x); \
	EXPECT_FLOAT_EQ(val1.y, val2.y); \
	EXPECT_FLOAT_EQ(val1.z, val2.z); \
	EXPECT_FLOAT_EQ(val1.w, val2.w)

using namespace rmlm;
using namespace rmlv;

TEST(Mat4, IdentityMultiplyGivesIdentity) {
	mat4 m = mat4::ident();
	vec4 a{ 3, 4, 5, 6 };
	auto result = m * a;
	auto expected = vec4{3,4,5,6};
	EXPECT_VEC4_EQ(result, expected); }


TEST(Mat43dScale, ScalesXYZNotW) {
	vec4 b{ 10, 10, 10, 1 };
	auto m = mat4::scale(vec4{ 1, 2, 3, 4 });  // w component should be dropped
	auto result = m * b;
	auto expected = vec4{ 10, 20, 30, 1};
	EXPECT_VEC4_EQ(result, expected); }

TEST(Mat43dTranslate, ChangeInZ) {
	auto a = vec4{ 3, 4, 5, 1 };
	auto m = mat4::translate(0, 0, -10);
	auto result = m * a;
	auto expected = vec4{ 3, 4, -5, 1 };
	EXPECT_VEC4_EQ(result, expected); }

TEST(Mat4, InitializeWithFloatsHasExpectedInternalStorage) {
	auto m = mat4{
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15 };

	// col 0
	EXPECT_FLOAT_EQ(m.ff[0], 0);
	EXPECT_FLOAT_EQ(m.ff[1], 4);
	EXPECT_FLOAT_EQ(m.ff[2], 8);
	EXPECT_FLOAT_EQ(m.ff[3], 12);

	// col 1
	EXPECT_FLOAT_EQ(m.ff[4], 1);
	EXPECT_FLOAT_EQ(m.ff[5], 5);
	EXPECT_FLOAT_EQ(m.ff[6], 9);
	EXPECT_FLOAT_EQ(m.ff[7], 13);

	// col 2
	EXPECT_FLOAT_EQ(m.ff[8], 2);
	EXPECT_FLOAT_EQ(m.ff[9], 6);
	EXPECT_FLOAT_EQ(m.ff[10], 10);
	EXPECT_FLOAT_EQ(m.ff[11], 14);

	// col 3
	EXPECT_FLOAT_EQ(m.ff[12], 3);
	EXPECT_FLOAT_EQ(m.ff[13], 7);
	EXPECT_FLOAT_EQ(m.ff[14], 11);
	EXPECT_FLOAT_EQ(m.ff[15], 15); }

}  // close unnamed namespace
