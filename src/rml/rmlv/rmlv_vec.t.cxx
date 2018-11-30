#include "src/rml/rmlv/rmlv_vec.hxx"

#include <iostream>
#include <string>
#include <gtest/gtest.h>
namespace {
using namespace rqdq::rmlv;

#define EXPECT_VEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x, val2.x); \
	EXPECT_FLOAT_EQ(val1.y, val2.y); \
	EXPECT_FLOAT_EQ(val1.z, val2.z); \
	EXPECT_FLOAT_EQ(val1.w, val2.w)

#define EXPECT_VEC3_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x, val2.x); \
	EXPECT_FLOAT_EQ(val1.y, val2.y); \
	EXPECT_FLOAT_EQ(val1.z, val2.z)

TEST(Vec4, Add) {
	vec4 a{ 1, 2, 3, 4 };
	vec4 b{ 12, 13, 14, 15 };
	vec4 res = a + b;
	EXPECT_FLOAT_EQ(res.x, 13);
	EXPECT_FLOAT_EQ(res.y, 15);
	EXPECT_FLOAT_EQ(res.z, 17);
	EXPECT_FLOAT_EQ(res.w, 19); }

TEST(Vec4, Subtract) {
	vec4 a{ 1, 2, 3, 4 };
	vec4 b{ 12, 13, 14, 15 };
	vec4 res = b - a;
	EXPECT_FLOAT_EQ(res.x, 11);
	EXPECT_FLOAT_EQ(res.y, 11);
	EXPECT_FLOAT_EQ(res.z, 11);
	EXPECT_FLOAT_EQ(res.w, 11); }

TEST(Vec4, Multiply) {
	vec4 a{ 1, 2, 3, 4 };
	vec4 b{ 12, 13, 14, 15 };
	vec4 res = b * a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 26);
	EXPECT_FLOAT_EQ(res.z, 42);
	EXPECT_FLOAT_EQ(res.w, 60); }

TEST(Vec4, Divide) {
	vec4 a{ 1, 2, 3, 4 };
	vec4 b{ 12, 13, 14, 15 };
	vec4 res = b / a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 6.5);
	EXPECT_FLOAT_EQ(res.z, 4.6666665);
	EXPECT_FLOAT_EQ(res.w, 3.75); }

TEST(Vec4, BroadcastInitializer) {
	vec4 one{ 1 };
	vec4 two{ 2 };
	vec4 res = one + two;
	EXPECT_FLOAT_EQ(res.x, 3);
	EXPECT_FLOAT_EQ(res.y, 3);
	EXPECT_FLOAT_EQ(res.z, 3);
	EXPECT_FLOAT_EQ(res.w, 3); }

TEST(Vec4, MutatedDefaultInit) {
	vec4 three{ 3 };
	vec4 started_default;
	started_default.x = -2;
	started_default.y = -3;
	started_default.z = -4;
	started_default.w = 10;
	vec4 res = three + started_default;
	EXPECT_FLOAT_EQ(res.x, 1);
	EXPECT_FLOAT_EQ(res.y, 0);
	EXPECT_FLOAT_EQ(res.z, -1);
	EXPECT_FLOAT_EQ(res.w, 13); }

/*TEST(Vec4, xyz0) {
	vec4 za{ 1, 2, 3, 4 };
	auto expected = vec4{1,2,3,0};
	EXPECT_VEC4_EQ(za.xyz0(), expected); }*/

TEST(Vec4, Negate) {
	vec4 d{ 2, 3, 4, 5};
	auto expected = vec4{-2,-3,-4,-5};
	EXPECT_VEC4_EQ(-d, expected); }



// vec3
TEST(Vec3, Add) {
	vec3 a{ 1, 2, 3 };
	vec3 b{ 12, 13, 14 };
	vec3 res = a + b;
	EXPECT_FLOAT_EQ(res.x, 13);
	EXPECT_FLOAT_EQ(res.y, 15);
	EXPECT_FLOAT_EQ(res.z, 17); }

TEST(Vec3, Subtract) {
	vec3 a{ 1, 2, 3 };
	vec3 b{ 12, 13, 14 };
	vec3 res = b - a;
	EXPECT_FLOAT_EQ(res.x, 11);
	EXPECT_FLOAT_EQ(res.y, 11);
	EXPECT_FLOAT_EQ(res.z, 11); }

TEST(Vec3, Multiply) {
	vec3 a{ 1, 2, 3 };
	vec3 b{ 12, 13, 14 };
	vec3 res = b * a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 26);
	EXPECT_FLOAT_EQ(res.z, 42); }

TEST(Vec3, Divide) {
	vec3 a{ 1, 2, 3 };
	vec3 b{ 12, 13, 14 };
	vec3 res = b / a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 6.5);
	EXPECT_FLOAT_EQ(res.z, 4.6666665); }

TEST(Vec3, BroadcastInitializer) {
	vec3 one{ 1 };
	vec3 two{ 2 };
	vec3 res = one + two;
	EXPECT_FLOAT_EQ(res.x, 3);
	EXPECT_FLOAT_EQ(res.y, 3);
	EXPECT_FLOAT_EQ(res.z, 3); }

TEST(Vec3, MutatedDefaultInit) {
	vec3 three{ 3 };
	vec3 started_default;
	started_default.x = -2;
	started_default.y = -3;
	started_default.z = -4;
	vec3 res = three + started_default;
	EXPECT_FLOAT_EQ(res.x, 1);
	EXPECT_FLOAT_EQ(res.y, 0);
	EXPECT_FLOAT_EQ(res.z, -1); }

// vec3
TEST(Vec2, Add) {
	vec2 a{ 1, 2 };
	vec2 b{ 12, 13 };
	vec2 res = a + b;
	EXPECT_FLOAT_EQ(res.x, 13);
	EXPECT_FLOAT_EQ(res.y, 15); }

TEST(Vec2, Subtract) {
	vec2 a{ 1, 2 };
	vec2 b{ 12, 13 };
	vec2 res = b - a;
	EXPECT_FLOAT_EQ(res.x, 11);
	EXPECT_FLOAT_EQ(res.y, 11); }

TEST(Vec2, Multiply) {
	vec2 a{ 1, 2 };
	vec2 b{ 12, 13 };
	vec2 res = b * a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 26); }

TEST(Vec2, Divide) {
	vec2 a{ 1, 2 };
	vec2 b{ 12, 13 };
	vec2 res = b / a;
	EXPECT_FLOAT_EQ(res.x, 12);
	EXPECT_FLOAT_EQ(res.y, 6.5); }

TEST(Vec2, BroadcastInitializer) {
	vec2 one{ 1 };
	vec2 two{ 2 };
	vec2 res = one + two;
	EXPECT_FLOAT_EQ(res.x, 3);
	EXPECT_FLOAT_EQ(res.y, 3); }

TEST(Vec2, MutatedDefaultInit) {
	vec2 three{ 3 };
	vec2 started_default;
	started_default.x = -2;
	started_default.y = -3;
	vec2 res = three + started_default;
	EXPECT_FLOAT_EQ(res.x, 1);
	EXPECT_FLOAT_EQ(res.y, 0); }


TEST(Hmax, Vec3) {
	EXPECT_FLOAT_EQ(hmax(vec3{10,20,30}), 30);
	EXPECT_FLOAT_EQ(hmax(vec3{30,20,10}), 30);
	EXPECT_FLOAT_EQ(hmax(vec3{10,30,20}), 30); }

TEST(Hmax, Vec4) {
	EXPECT_FLOAT_EQ(hmax(vec4{10,20,30,40}), 40);
	EXPECT_FLOAT_EQ(hmax(vec4{40,30,20,10}), 40);
	EXPECT_FLOAT_EQ(hmax(vec4{10,40,30,20}), 40);
	EXPECT_FLOAT_EQ(hmax(vec4{10,30,40,20}), 40); }

TEST(Hmin, Vec3) {
	EXPECT_FLOAT_EQ(hmin(vec3{10,20,30}), 10);
	EXPECT_FLOAT_EQ(hmin(vec3{30,20,10}), 10);
	EXPECT_FLOAT_EQ(hmin(vec3{20,10,30}), 10); }

TEST(Hmin, Vec4) {
	EXPECT_FLOAT_EQ(hmin(vec4{10,20,30,40}), 10);
	EXPECT_FLOAT_EQ(hmin(vec4{30,20,10,40}), 10);
	EXPECT_FLOAT_EQ(hmin(vec4{20,10,30,40}), 10);
	EXPECT_FLOAT_EQ(hmin(vec4{20,40,30,10}), 10); }

TEST(Length, Vec4) {
	EXPECT_FLOAT_EQ(length(vec4{4,4,0,0}), 5.6568542);
	EXPECT_FLOAT_EQ(length(vec4{1,1,1,0}), 1.7320508);
	EXPECT_FLOAT_EQ(length(vec4{1,1,1,1}), 2); }

TEST(Abs, Vec4) {
	auto foo = vec4{-10,-20,30,-40};
	auto expected = vec4{10,20,30,40};
	EXPECT_VEC4_EQ(abs(foo), expected);

	foo = vec4{100,-234,0,3.14};
	expected = vec4{100,234,0,3.14};
	EXPECT_VEC4_EQ(abs(foo), expected); }

TEST(Dot, Vec4) {
	vec4 da{ 2, 3, 4, 0 };
	vec4 db{ 3, 4, 5, 0 };
	EXPECT_FLOAT_EQ(dot(da, db), 3*2+4*3+5*4+0*0); }

TEST(Sqrt, Vec4) {
	vec4 foo{ 16 };
	auto expected = vec4{4,4,4,4};
	EXPECT_VEC4_EQ(sqrt(foo), expected); }

TEST(Normalize, Vec4) {
	vec4 h{ 10,10,10,0 };
	auto expected_normal = vec4{ 0.57735026, 0.57735026, 0.57735026, 0 };
	EXPECT_VEC4_EQ(normalize(h), expected_normal);
	EXPECT_FLOAT_EQ(length(normalize(h)), 1.0); }

TEST(Hadd, Vec4) {
	vec4 i{ 4, 13, 8, -10 };
	EXPECT_FLOAT_EQ(hadd(i), 4 + 13 + 8 + (-10)); }

TEST(Cross, Vec4) {
	vec4 ca{ 3, 4, 5, 0 };
	vec4 cb{ 4, 5, 6, 0 };
	auto expected = vec4{ -1, 2, -1,0 };
	EXPECT_VEC4_EQ(cross(ca,cb), expected);
	cb = vec4{ 4, 5, -6, 0 };
	expected = vec4{ -49, 38, -1, 0 };
	EXPECT_VEC4_EQ(cross(ca, cb), expected); }

} // close unnamed namespace
