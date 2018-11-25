#include <rglv_packed_stream.hxx>
#include <rmlv_vec.hxx>

#include <iostream>

#include <gtest/gtest.h>

using namespace rqdq;
using namespace std;

namespace {

TEST(PackedStream, BasicOperation) {
	rglv::FastPackedStream ps;
	ps.appendByte(11);
	ps.appendInt(0x7f7f7f7f);
	ps.appendVec4({ 1.1, 2.2, 3.3, 4.4 });
	ps.appendByte(44);
	ps.appendByte(8);
	ps.appendByte(38);

	EXPECT_EQ(ps.consumeByte(), 11);
	EXPECT_EQ(ps.consumeInt(), 0x7f7f7f7f);

	auto v = ps.consumeVec4();
	EXPECT_FLOAT_EQ(v.x, 1.1);
	EXPECT_FLOAT_EQ(v.y, 2.2);
	EXPECT_FLOAT_EQ(v.z, 3.3);
	EXPECT_FLOAT_EQ(v.w, 4.4);

	EXPECT_EQ(ps.consumeByte(), 44);
	EXPECT_EQ(ps.consumeByte(), 8);
	EXPECT_EQ(ps.consumeByte(), 38); }

}  // close unnamed namespace
