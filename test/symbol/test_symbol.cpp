// Copyright (c) 2016, Cycling '74
// Timothy Place
// Usage of this file and its contents is governed by the MIT License

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "c74_mock.h"
#include "c74_min.h"


TEST_CASE( "Symbol Class", "[symbols]" ) {

    SECTION("symbol assignments") {
		c74::min::symbol s1 = "foo";
		const char* c1 = s1;
		
		REQUIRE( s1 == "foo" );
		REQUIRE( !strcmp(c1, "foo") );
	}

}
