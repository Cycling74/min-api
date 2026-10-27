/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.
#include "catch.hpp"
#include "c74_min_api.h"

extern "C" long mock_refcount(void* x);
extern "C" long mock_dictobj_release_count();

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

	SECTION("copying an owning dict retains the shared dictionary") {
		auto raw = max::dictionary_new();
		{
			min::dict a { raw };
			{
				min::dict b { a };
				REQUIRE( mock_refcount(raw) == 3 );
			}
			REQUIRE( mock_refcount(raw) == 2 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("copying a borrowing dict also borrows") {
		auto raw = max::dictionary_new();
		{
			min::dict a { raw, false };
			min::dict b { a };
			REQUIRE( mock_refcount(raw) == 1 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("moving a dict transfers its reference") {
		auto raw = max::dictionary_new();
		{
			min::dict a { raw };
			min::dict b { std::move(a) };
			REQUIRE( mock_refcount(raw) == 2 );
			REQUIRE( !a.valid() );
			REQUIRE( b.valid() );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("assigning an atom copies contents without changing either dictionary's refcount") {
		auto target = max::dictionary_new();
		auto source = max::dictionary_new();
		{
			min::dict d { target };
			min::atom a { source };
			d = a;
			REQUIRE( mock_refcount(target) == 2 );
			REQUIRE( mock_refcount(source) == 1 );
		}
		REQUIRE( mock_refcount(target) == 1 );
		REQUIRE( mock_refcount(source) == 1 );
		max::object_free(target);
		max::object_free(source);
	}

	SECTION("instance() borrows and casting to t_object* retains") {
		auto raw = max::dictionary_new();
		{
			min::dict d { raw };
			REQUIRE( d.instance() == raw );
			REQUIRE( mock_refcount(raw) == 2 );

			auto o = static_cast<max::t_object*>(d);
			REQUIRE( mock_refcount(raw) == 3 );
			max::object_free(o);
		}
		REQUIRE( mock_refcount(raw) == 1 );
		max::object_free(raw);
	}

	SECTION("a dict found by name is released with dictobj_release") {
		auto raw = max::dictionary_new();
		max::t_symbol* name = min::symbol("dict_found_by_name");
		max::dictobj_register(raw, &name);

		auto releases = mock_dictobj_release_count();
		{
			min::dict d { min::symbol("dict_found_by_name") };
			REQUIRE( d.instance() == raw );
			REQUIRE( mock_refcount(raw) == 2 );
		}
		REQUIRE( mock_refcount(raw) == 1 );
		REQUIRE( mock_dictobj_release_count() == releases + 1 );
		max::object_free(raw);
	}

	SECTION("copies of a dict found by name are released with object_free and moves with dictobj_release") {
		auto raw = max::dictionary_new();
		max::t_symbol* name = min::symbol("dict_found_by_name_copied");
		max::dictobj_register(raw, &name);

		auto releases = mock_dictobj_release_count();
		{
			min::dict a { min::symbol("dict_found_by_name_copied") };
			{
				min::dict b { a };
				REQUIRE( mock_refcount(raw) == 3 );
			}
			REQUIRE( mock_dictobj_release_count() == releases );
			REQUIRE( mock_refcount(raw) == 2 );

			min::dict c { std::move(a) };
		}
		REQUIRE( mock_refcount(raw) == 1 );
		REQUIRE( mock_dictobj_release_count() == releases + 1 );
		max::object_free(raw);
	}

	SECTION("a dict created under a new name is released with object_free") {
		auto releases = mock_dictobj_release_count();
		{
			min::dict d { min::symbol("dict_created_by_name") };
			REQUIRE( mock_refcount(d.instance()) == 1 );
		}
		REQUIRE( mock_dictobj_release_count() == releases );
	}

	SECTION("empty dict-syntax creates an empty dictionary") {
		min::dict d { min::atoms{} };
		REQUIRE( d.valid() );
		REQUIRE( mock_refcount(d.instance()) == 1 );
	}
}
