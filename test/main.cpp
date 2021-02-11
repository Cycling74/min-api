#define CATCH_CONFIG_MAIN

#include "c74_min_catch.h"

using namespace c74::min;

class TestObject : public object<TestObject> {};

TEST_CASE("Attribute ranges") {
	TestObject my_object;
	attribute<number, threadsafe::no, limit::clamp> my_attr {&my_object, "My Attribute", 0.0, range {-10.0, 10.0} };
	
	SECTION("Cannot set attribute to a value outside its range") {
		const auto value = GENERATE(-100.0, 25.0, 11.5);
		my_attr = value;
		REQUIRE(static_cast<number>(my_attr) >= -10.0);
		REQUIRE(static_cast<number>(my_attr) <= 10.0);
	}
}
