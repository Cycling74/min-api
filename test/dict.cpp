/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.
#include "catch.hpp"
#include "c74_min_api.h"

extern "C" long mock_refcount(void* x);

using namespace c74;

TEST_CASE("Dict Class", "[dict]") {

	SECTION("retaining an existing dictionary releases it on destruction") {
		auto raw = max::dictionary_new();
		{
			min::dict d { raw };
			REQUIRE( mock_refcount(raw) == 2 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("borrowing an existing dictionary leaves its refcount alone") {
		auto raw = max::dictionary_new();
		{
			min::dict d { raw, false };
			REQUIRE( mock_refcount(raw) == 1 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("a dict holding an atom with a dictionary retains it") {
		auto raw = max::dictionary_new();
		{
			min::atom a { raw };
			min::dict d { a };
			REQUIRE( mock_refcount(raw) == 2 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}
}
