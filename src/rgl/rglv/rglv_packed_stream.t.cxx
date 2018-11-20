#include <rglv_packed_stream.hxx>
#include <rmlv_vec.hxx>

#include <iostream>
#include <fmt/printf.h>

using namespace rqdq;
using namespace std;

int main() {
	rglv::FastPackedStream ps;
	ps.appendByte(11);
	ps.appendInt(0x7f7f7f7f);
	ps.appendVec4({ 1.1, 2.2, 3.3, 4.4 });
	ps.appendByte(44);
	ps.appendByte(8);
	ps.appendByte(38);

	auto xa = ps.consumeByte();
	auto xb = ps.consumeInt();
	auto xc = ps.consumeVec4();
	auto xd = ps.consumeByte();
	auto xe = ps.consumeByte();
	auto xf = ps.consumeByte();
	fmt::printf("%d,", xa);
	fmt::printf("%08x,", xb);
	cout << xc;
	fmt::printf("%d,", xd);
	fmt::printf("%d,", xe);
	fmt::printf("%d,", xf);
}
