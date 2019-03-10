#include <string>
#include <iostream>

#include "src/rgl/rglv/rglv_math.hxx"
#include "src/rml/rmlm/rmlm_mat4.hxx"
#include "src/rml/rmlv/rmlv_vec.hxx"

#include <gtest/gtest.h>

#define EXPECT_VEC4_EQ(val1, val2) \
	EXPECT_FLOAT_EQ(val1.x, val2.x);\
	EXPECT_FLOAT_EQ(val1.y, val2.y);\
	EXPECT_FLOAT_EQ(val1.z, val2.z);\
	EXPECT_FLOAT_EQ(val1.w, val2.w)

namespace {

using namespace rqdq;
using namespace std;

TEST(MakeDeviceMatrix, TopLeftMatchesPixelTopLeft) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	rmlv::vec4 top_left{ -1.0, 1.0, 0, 1.0f };
	string test_name{ "top-left" };
	auto value = device_matrix * top_left;
	auto expected = rmlv::vec4{ 0,0,0,1 };
	EXPECT_VEC4_EQ(value, expected); }

TEST(MakeDeviceMatrix, BottomLeftMatchesPixelBottomLeft) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	rmlv::vec4 bottom_left{ -1.0, -1.0, 0, 1.0f };
	string test_name{ "bottom-left" };
	auto value = device_matrix * bottom_left;
	auto expected = rmlv::vec4{ 0,240,0,1 };
	EXPECT_VEC4_EQ(value, expected); }

TEST(MakeDeviceMatrix, TopRightMatchesPixelTopRight) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	rmlv::vec4 top_right{1.0, 1.0, 0, 1 };
	string test_name{ "top-right" };
	auto value = device_matrix * top_right;
	auto expected = rmlv::vec4{ 320,0,0,1 };
	EXPECT_VEC4_EQ(value, expected); }

TEST(MakeDeviceMatrix, BottomRightMatchesPixelBottomRight) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	rmlv::vec4 bottom_right{ 1.0, -1.0, 0, 1.0f };
	string test_name{ "bottom-right" };
	auto value = device_matrix * bottom_right;
	auto expected = rmlv::vec4{ 320,240,0,1 };
	EXPECT_VEC4_EQ(value, expected); }


// test ortho using a pixel-center projection
// https://stackoverflow.com/questions/14608395/pixel-perfect-2d-rendering-with-opengl
TEST(MakeOrthographicMatrix, TopLeftPixelCenter) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	auto projection_matrix = rglv::make_glOrtho(-0.5, (320 - 1) + 0.5, (240 - 1) + 0.5, -0.5, -1.0, 1.0);
	rmlv::vec4 point{ 0.0, 0.0, 0, 1.0f };
	string test_name{ "orth top left" };
	auto value = projection_matrix * point;
	value = device_matrix * value;
	auto expected = rmlv::vec4{ 0.5,0.5,0,1 };
	EXPECT_VEC4_EQ(value, expected); }

TEST(MakeOrthographicMatrix, BottomRightPixelCenter) {
	auto device_matrix = rglv::make_device_matrix(320, 240);
	auto projection_matrix = rglv::make_glOrtho(-0.5, (320 - 1) + 0.5, (240 - 1) + 0.5, -0.5, -1.0, 1.0);
	rmlv::vec4 point{ 319.0, 239.0, 0, 1.0f };
	string test_name{ "ortho bottom right" };
	auto value = projection_matrix * point;
	value = device_matrix * value;
	auto expected = rmlv::vec4{ 319.5,239.5,0,1 };
	EXPECT_VEC4_EQ(value, expected); }


TEST(PerspectiveDivide, BasicOperation) {
	rmlv::vec4 foo{ 10,20,30,10 };
	rmlv::vec4 expected{ 1, 2, 3, 1.0f / 10.0f };
	EXPECT_VEC4_EQ(rglv::pdiv(foo), expected); }

}  // close unnamed namespce
