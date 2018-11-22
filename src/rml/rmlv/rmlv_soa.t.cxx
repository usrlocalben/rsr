#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>

#include <iostream>
#include <string>
#include <gtest/gtest.h>

namespace {

using namespace rqdq;
using rmlv::vec4;
using rmlv::mvec4f;

#define EXPECT_MVEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.get_x(), val2.x); \
	EXPECT_FLOAT_EQ(val1.get_y(), val2.y); \
	EXPECT_FLOAT_EQ(val1.get_z(), val2.z); \
	EXPECT_FLOAT_EQ(val1.get_w(), val2.w)

TEST(Vec4, Shuffles) {
	mvec4f a{ 2, 3, 4, 5 };
	mvec4f res;

	res = a.xxxx();
	auto _2222 = vec4{2,2,2,2};
	EXPECT_MVEC4_EQ(res, _2222);
	res = a.yyyy();
	auto _3333 = vec4{3,3,3,3};
	EXPECT_MVEC4_EQ(res, _3333);
	res = a.zzzz();
	auto _4444 = vec4{4,4,4,4};
	EXPECT_MVEC4_EQ(res, _4444);
	res = a.wwww();
	auto _5555 = vec4{5,5,5,5};
	EXPECT_MVEC4_EQ(res, _5555);

	res = a.yzxw();
	auto _3425 = vec4{3,4,2,5};
	EXPECT_MVEC4_EQ(res, _3425);
	res = a.zxyw();
	auto _4235 = vec4{4,2,3,5};
	EXPECT_MVEC4_EQ(res, _4235);

	res = a.xyxy();
	auto _2323 = vec4{2,3,2,3};
	EXPECT_MVEC4_EQ(res, _2323);
	res = a.zwzw();
	auto _4545 = vec4{4,5,4,5};
	EXPECT_MVEC4_EQ(res, _4545); }

}  // close unnamed namespace
