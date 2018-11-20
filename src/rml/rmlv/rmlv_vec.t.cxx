#include <rmlv_vec.hxx>

#include <iostream>
#include <string>


void ok() {
	std::cout << "."; }

void fail() {
	std::cout << "F"; }


void assertEqual(const std::string& a, const std::string& b) {
	if (a == b) {
		ok();
		return; }
	std::cout << "error, strings not equal " << a << " != " << b << std::endl; }


void assertAlmostEqual(float a, float b) {
	static const auto ep = 0.001f;
	if (abs(a - b) < ep) {
		ok();
		return; }
	std::cout << "error, not almost equal " << a << ", !~= " << b << std::endl; }


void assertAlmostEqual(const rmlv::vec4& a, const rmlv::vec4& b) {
	static const auto ep = 0.001f;
	float dx = abs(a.x - b.x);
	float dy = abs(a.y - b.y);
	float dz = abs(a.z - b.z);
	float dw = abs(a.w - b.w);
	if (dx < ep && dy < ep && dz < ep && dw < ep) {
		ok();
		return; }
	std::cout << "error, not almost equal " << a << ", !~= " << b << std::endl; }


void test_vec4_basic_math() {
	rmlv::vec4 a{ 1, 2, 3, 4 };
	rmlv::vec4 b{ 12, 13, 14, 15 };
	rmlv::vec4 res = a + b;

//	std::cout << "test basic math 1" << std::endl;
	assertAlmostEqual(res.x, 13);
	assertAlmostEqual(res.y, 15);
	assertAlmostEqual(res.z, 17);
	assertAlmostEqual(res.w, 19);

	res = b - a;
//	std::cout << "test basic math 2" << std::endl;
	assertAlmostEqual(res.x, 11);
	assertAlmostEqual(res.y, 11);
	assertAlmostEqual(res.z, 11);
	assertAlmostEqual(res.w, 11);

	res = b * a;
//	std::cout << "test basic math 3" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 26);
	assertAlmostEqual(res.z, 42);
	assertAlmostEqual(res.w, 60);

	res = b / a;
//	std::cout << "test basic math 4" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 6.5);
	assertAlmostEqual(res.z, 4.666);
	assertAlmostEqual(res.w, 3.75);

	rmlv::vec4 one{ 1 };
	rmlv::vec4 two{ 2 };
	res = one + two;
//	std::cout << "test basic math 5" << std::endl;
	assertAlmostEqual(res.x, 3);
	assertAlmostEqual(res.y, 3);
	assertAlmostEqual(res.z, 3);
	assertAlmostEqual(res.w, 3);

	rmlv::vec4 three{ 3 };
	rmlv::vec4 started_default;

	started_default.x = -2;
	started_default.y = -3;
	started_default.z = -4;
	started_default.w = 10;
	res = three + started_default;
//	std::cout << "test basic math 6" << std::endl;
	assertAlmostEqual(res.x, 1);
	assertAlmostEqual(res.y, 0);
	assertAlmostEqual(res.z, -1);
	assertAlmostEqual(res.w, 13);
	
	std::cout << std::endl;
}


void test_vec3_basic_math() {
	rmlv::vec3 a{ 1, 2, 3 };
	rmlv::vec3 b{ 12, 13, 14 };
	rmlv::vec3 res = a + b;

//	std::cout << "test basic math 1" << std::endl;
	assertAlmostEqual(res.x, 13);
	assertAlmostEqual(res.y, 15);
	assertAlmostEqual(res.z, 17);

	res = b - a;
//	std::cout << "test basic math 2" << std::endl;
	assertAlmostEqual(res.x, 11);
	assertAlmostEqual(res.y, 11);
	assertAlmostEqual(res.z, 11);

	res = b * a;
//	std::cout << "test basic math 3" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 26);
	assertAlmostEqual(res.z, 42);

	res = b / a;
//	std::cout << "test basic math 4" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 6.5);
	assertAlmostEqual(res.z, 4.666);

	rmlv::vec3 one{ 1 };
	rmlv::vec3 two{ 2 };
	res = one + two;
//	std::cout << "test basic math 5" << std::endl;
	assertAlmostEqual(res.x, 3);
	assertAlmostEqual(res.y, 3);
	assertAlmostEqual(res.z, 3);

	rmlv::vec3 three{ 3 };
	rmlv::vec3 started_default;

	started_default.x = -2;
	started_default.y = -3;
	started_default.z = -4;
	res = three + started_default;
//	std::cout << "test basic math 6" << std::endl;
	assertAlmostEqual(res.x, 1);
	assertAlmostEqual(res.y, 0);
	assertAlmostEqual(res.z, -1);
	
	std::cout << std::endl;
}


void test_vec2_basic_math() {
	rmlv::vec2 a{ 1, 2 };
	rmlv::vec2 b{ 12, 13 };
	rmlv::vec2 res = a + b;

//	std::cout << "test basic math 1" << std::endl;
	assertAlmostEqual(res.x, 13);
	assertAlmostEqual(res.y, 15);

	res = b - a;
//	std::cout << "test basic math 2" << std::endl;
	assertAlmostEqual(res.x, 11);
	assertAlmostEqual(res.y, 11);

	res = b * a;
//	std::cout << "test basic math 3" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 26);

	res = b / a;
//	std::cout << "test basic math 4" << std::endl;
	assertAlmostEqual(res.x, 12);
	assertAlmostEqual(res.y, 6.5);

	rmlv::vec2 one{ 1 };
	rmlv::vec2 two{ 2 };
	res = one + two;
//	std::cout << "test basic math 5" << std::endl;
	assertAlmostEqual(res.x, 3);
	assertAlmostEqual(res.y, 3);

	rmlv::vec2 three{ 3 };
	rmlv::vec2 started_default;

	started_default.x = -2;
	started_default.y = -3;
	res = three + started_default;
//	std::cout << "test basic math 6" << std::endl;
	assertAlmostEqual(res.x, 1);
	assertAlmostEqual(res.y, 0);
	
	std::cout << std::endl;
}


/*void test_vec4_shuffles() {
	mvec4f a{ 2, 3, 4, 5 };
	mvec4f res;

	res = a.xxxx();
	assertAlmostEqual(res, vec4{ 2,2,2,2 });
	res = a.yyyy();
	assertAlmostEqual(res, vec4{ 3,3,3,3 });
	res = a.zzzz();
	assertAlmostEqual(res, vec4{ 4,4,4,4 });
	res = a.wwww();
	assertAlmostEqual(res, vec4{ 5,5,5,5 });

	res = a.yzxw();
	assertAlmostEqual(res, vec4{ 3, 4, 2, 5 });
	res = a.zxyw();
	assertAlmostEqual(res, vec4{ 4, 2, 3, 5 });

	res = a.xyxy();
	assertAlmostEqual(res, vec4{ 2, 3, 2, 3 });
	res = a.zwzw();
	assertAlmostEqual(res, vec4{ 4, 5, 4, 5 });
	std::cout << std::endl;
}*/


void test_vec3_more() {
	assertAlmostEqual(hmax(rmlv::vec3{ 10,20,30 }), 30);
	assertAlmostEqual(hmax(rmlv::vec3{ 30,20,10 }), 30);
	assertAlmostEqual(hmax(rmlv::vec3{ 10,30,20 }), 30);
	assertAlmostEqual(hmin(rmlv::vec3{ 10,20,30 }), 10);
	assertAlmostEqual(hmin(rmlv::vec3{ 30,20,10 }), 10);
	assertAlmostEqual(hmin(rmlv::vec3{ 10,30,20 }), 10);
}

/*void test_vec4_more() {
	vec4 a{ 4, 4, 0, 0 };
	auto len = length(a);

	assertAlmostEqual(len, 5.656);

	vec4 b{ 1, 1, 1, 0 };
	len = length(b);
	assertAlmostEqual(len, 1.732);

	vec4 c{ 1, 1, 1, 1 };
	len = length(c);
	assertAlmostEqual(len, 2);

	vec4 d{ 2, 3, 4, 5 };
	vec4 neg = -d;
	assertAlmostEqual(neg, vec4{ -2, -3, -4, -5 });

	vec4 e{ -10, -20, 30, -40 };
	vec4 f{ 100, -234, 0, 3.14f };
	assertAlmostEqual(abs(e), vec4{ 10, 20, 30, 40 });
	assertAlmostEqual(abs(f), vec4{ 100, 234, 0, 3.14f });

	vec4 da{ 2, 3, 4, 0 };
	vec4 db{ 3, 4, 5, 0 };
	assertAlmostEqual(dot(da, db), 3 * 2 + 4 * 3 + 5 * 4);

	vec4 g{ 16 };
	assertAlmostEqual(sqrt(g), vec4{ 4,4,4,4 });

	vec4 h{ 10,10,10,0 };
	assertAlmostEqual(normalize(h), vec4{ 0.5772f, 0.5772f, 0.5772f, 0 });
	assertAlmostEqual(length(normalize(h)), 1.0);

	vec4 i{ 4, 13, 8, -10 };
	assertAlmostEqual(hadd(i), 4 + 13 + 8 + (-10));
	assertAlmostEqual(hmin(i), -10);
	assertAlmostEqual(hmax(i), 13);

	assertAlmostEqual(hmax(vec4{ 10,20,30,40 }), 40);
	assertAlmostEqual(hmax(vec4{ 30,20,40,10 }), 40);
	assertAlmostEqual(hmax(vec4{ 40,30,20,10 }), 40);
	assertAlmostEqual(hmax(vec4{ 30,40,20,10 }), 40);
	assertAlmostEqual(hmin(vec4{ 10,20,30,40 }), 10);
	assertAlmostEqual(hmin(vec4{ 30,20,40,10 }), 10);
	assertAlmostEqual(hmin(vec4{ 10,40,20,30 }), 10);
	assertAlmostEqual(hmin(vec4{ 40,10,20,30 }), 10);

	vec4 ca{ 3, 4, 5, 0 };
	vec4 cb{ 4, 5, 6, 0 };
	assertAlmostEqual(cross(ca,cb), vec4{ -1, 2, -1,0 });
	cb = vec4{ 4, 5, -6, 0 };
	assertAlmostEqual(cross(ca, cb), vec4{ -49, 38, -1, 0 });
	std::cout << std::endl;


	vec4 pa{ 10,20,30,10 };
	assertAlmostEqual(perspective_divide(pa), vec4{ 1,2,3,1.0f / 10.0f });

	vec4 za{ 1, 2, 3, 4 };
	assertAlmostEqual(za.xyz0(), vec4{ 1,2,3,0 });
}*/


int main() {
	test_vec4_basic_math();
	test_vec3_basic_math();
	test_vec2_basic_math();
//	test_vec4_shuffles();
//	test_vec4_more();
	return 0; }
