#include <rmlv_soa.hxx>
#include <rmlv_vec.hxx>

#include <iostream>
#include <string>
#include <gtest/gtest.h>

namespace {

using namespace rqdq;

#define EXPECT_VEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x, val2.x); \
	EXPECT_FLOAT_EQ(val1.y, val2.y); \
	EXPECT_FLOAT_EQ(val1.z, val2.z); \
	EXPECT_FLOAT_EQ(val1.w, val2.w)

#define EXPECT_MVEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x(), val2.x); \
	EXPECT_FLOAT_EQ(val1.y(), val2.y); \
	EXPECT_FLOAT_EQ(val1.z(), val2.z); \
	EXPECT_FLOAT_EQ(val1.w(), val2.w)

TEST(Vec4, Shuffles) {
	using rmlv::vec4;
	using rmlv::mvec4f;
	mvec4f a{ 2, 3, 4, 5 };
	mvec4f res;

	res = a.xxxx();
	EXPECT_MVEC4_EQ(res, vec4{ 2,2,2,2 });
	res = a.yyyy();
	EXPECT_MVEC4_EQ(res, vec4{ 3,3,3,3 });
	res = a.zzzz();
	EXPECT_MVEC4_EQ(res, vec4{ 4,4,4,4 });
	res = a.wwww();
	EXPECT_MVEC4_EQ(res, vec4{ 5,5,5,5 });

	res = a.yzxw();
	EXPECT_MVEC4_EQ(res, vec4{ 3, 4, 2, 5 });
	res = a.zxyw();
	EXPECT_MVEC4_EQ(res, vec4{ 4, 2, 3, 5 });

	res = a.xyxy();
	EXPECT_MVEC4_EQ(res, vec4{ 2, 3, 2, 3 });
	res = a.zwzw();
	EXPECT_MVEC4_EQ(res, vec4{ 4, 5, 4, 5 });
}

}  // close unnamed namespace
