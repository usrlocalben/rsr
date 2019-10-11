#include "src/rml/rmlv/rmlv_soa.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

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


TEST(Vec4, Wrap1) {
	using rmlv::mvec4f;
	EXPECT_FLOAT_EQ(wrap1(mvec4f{0.0F}).get_x(), 0.0F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{0.999}).get_x(), 0.999F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{1.0F}).get_x(), 1.0F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{1.1F}).get_x(), -0.9F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{20.0F}).get_x(), 0.0F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-0.5F}).get_x(), -0.5F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-0.999}).get_x(), -0.999F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-1.0F}).get_x(), -1.0F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-1.2F}).get_x(),  0.8F);

	EXPECT_NEAR    (wrap1(mvec4f{-2.1F}).get_x(), -0.1F, 0.000001);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-1.1F}).get_x(),  0.9F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{-0.1F}).get_x(), -0.1F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{ 0.9F}).get_x(), 0.9F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{ 1.9F}).get_x(),-0.1F);
	EXPECT_FLOAT_EQ(wrap1(mvec4f{ 2.9F}).get_x(), 0.9F);
	EXPECT_NEAR    (wrap1(mvec4f{ 3.9F}).get_x(),-0.1F, 0.000001); }


TEST(Vec4, SinCos) {
	using rmlv::mvec4f;
	for (float x = -10000.0F; x<10000.0F; x+=0.1F) {
		EXPECT_NEAR(sin(mvec4f(x)).get_x(), std::sinf(x), 0.00250);
		EXPECT_NEAR(cos(mvec4f(x)).get_x(), std::cosf(x), 0.00250); } }


}  // close unnamed namespace
