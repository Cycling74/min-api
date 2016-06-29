// Copyright (c) 2016, Cycling '74
// Timothy Place
// Usage of this file and its contents is governed by the MIT License

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "c74_min.h"


SCENARIO ("classname deduction is called") {

    WHEN ("a name is specified directly (not a fullpath or file with extension)") {
		THEN ("the name remains unmodified, even if containing dots") {
			auto str = c74::min::deduce_maxclassname("ableton.foo~");
			REQUIRE( str == "ableton.foo~" );
		}
	}

    AND_WHEN ("a fullpath from the __FILE__ macro is used, formatted for the Mac") {
		THEN ("the name is extracted, stripping the path and the source file extension") {
			auto str = c74::min::deduce_maxclassname("/Users/tim/Materials/min-devkit/source/projects/randfloat/randfloat.cpp");
			REQUIRE( str == "randfloat" );
		}
		AND_THEN ("it also works if there are spaces in the path") {
			auto str = c74::min::deduce_maxclassname("/Users/tim/my code/min-devkit/source/projects/randfloat/randfloat.cpp");
			REQUIRE( str == "randfloat" );
		}
		AND_THEN ("the _tilde is properly substituted for MSP objects") {
			auto str = c74::min::deduce_maxclassname("/Users/tim/Materials/min-devkit/source/projects/lores_tilde/lores_tilde.cpp");
			REQUIRE( str == "lores~" );
		}
		AND_THEN ("combined dots and _tilde also produce correct results") {
			auto str = c74::min::deduce_maxclassname("/Users/tim/Materials/min-devkit/source/projects/jit.dict.oscil_tilde/jit.dict.oscil_tilde.cpp");
			REQUIRE( str == "jit.dict.oscil~" );
		}
	}

    AND_WHEN ("a fullpath from the __FILE__ macro is used, formatted for Windows") {
		THEN ("the name is extracted, stripping the path and the source file extension") {
			auto str = c74::min::deduce_maxclassname("C:\\Users\\tim\\Documents\\Max 7\\Packages\\min-devkit\\source\\projects\\banger\\banger.cpp");
			REQUIRE( str == "banger" );
		}
		AND_THEN ("the _tilde is properly substituted for MSP objects") {
			auto str = c74::min::deduce_maxclassname("C:\\Users\\tim\\Documents\\Max 7\\Packages\\min-devkit\\source\\projects\\lores\\lores_tilde.cpp");
			REQUIRE( str == "lores~" );
		}
		AND_THEN ("combined dots and _tilde also produce correct results") {
			auto str = c74::min::deduce_maxclassname("C:\\Users\\tim\\Documents\\Max 7\\Packages\\min-devkit\\source\\projects\\jit.dict.oscil_tilde\\jit.dict.oscil_tilde.cpp");
			REQUIRE( str == "jit.dict.oscil~" );
		}
	}

}
