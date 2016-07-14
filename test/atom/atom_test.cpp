// Copyright (c) 2016, Cycling '74
// Timothy Place
// Usage of this file and its contents is governed by the MIT License

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "c74_min.h"


TEST_CASE( "Atom Class", "[atoms]" ) {

    SECTION("to_atoms() function") {
		std::vector<double>	dv = { 1, 2.0, -3.14, 4.5};
		c74::min::atoms		as = c74::min::to_atoms(dv);
				
		REQUIRE( as.size() == 4 );
		REQUIRE( double(as[0]) == 1.0 );
		REQUIRE( double(as[1]) == 2.0 );
		REQUIRE( double(as[2]) == -3.14 );
		REQUIRE( double(as[3]) == 4.5 );
	}


    SECTION("from_atoms() function") {
		c74::min::atoms		as = { 1, 2.0, -3.14, 4.5};
		std::vector<double>	dv = c74::min::from_atoms<std::vector<double>>(as);
				
		REQUIRE( dv.size() == 4 );
		REQUIRE( dv[0] == 1.0 );
		REQUIRE( dv[1] == 2.0 );
		REQUIRE( dv[2] == -3.14 );
		REQUIRE( dv[3] == 4.5 );
	}

}
