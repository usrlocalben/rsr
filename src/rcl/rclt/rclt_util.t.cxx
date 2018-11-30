#include "src/rcl/rclt/rclt_util.hxx"

#include <iostream>
#include <string>
#include <gtest/gtest.h>

using namespace rqdq;

namespace {

TEST(Trim, TrimsWhitespacePrefix) {
	EXPECT_EQ(rclt::trim("   hey"), "hey"); }

TEST(Trim, TrimsWhitespaceSuffix) {
	EXPECT_EQ(rclt::trim("hey   "), "hey"); }

TEST(Trim, TrimsWhitespaceBoth) {
	EXPECT_EQ(rclt::trim("   hey   "), "hey"); }

TEST(Trim, NoWhitespaceIsIdentity) {
	EXPECT_EQ(rclt::trim("hey"), "hey"); }

}  // close unnamed namespace
