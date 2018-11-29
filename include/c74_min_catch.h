/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

#include "catch.hpp"    // The Catch header must come first -- otherwise some C++ includes will tangle with it and cause problems.
#include "c74_min.h"    // The standard Min header

/// Compare a container (e.g. a vector) of floats in a Catch unit test.
/// If there is a failure, not only will the return value be false, but a Catch REQUIRE will fail.
///
/// @tparam T           The type for comparisons. Should be an STL container of floats or doubles.
/// @param  source      The values to compare
/// @param  reference   The reference values against which to compare.
/// @return             True if they are (approximately) the same. Otherwise false.

template<typename T>
bool require_vector_approx(T source, T reference) {
	REQUIRE(source.size() == reference.size());
	if (source.size() != reference.size())
		return false;

	for (auto i = 0; i < source.size(); ++i) {
		auto e = std::numeric_limits<float>::epsilon()*1000;
		INFO("when i == " << i);
		REQUIRE(source[i] == Approx(reference[i]).epsilon(e).margin(e));
		if (source[i] != Approx(reference[i]).epsilon(e).margin(e))
			return false;
	}
	return true;
}


/// Compare a container (e.g. a vector) of floats in a Catch unit test.
/// If there is a failure, not only will the return value be false, but a Catch REQUIRE will fail.
///
/// @tparam T           The type for comparisons. Should be an STL container of floats or doubles.
/// @param  source      The values to compare
/// @param  reference   The reference values against which to compare.
/// @return             True if they are (approximately) the same. Otherwise false.

#define REQUIRE_VECTOR_APPROX(source, reference) require_vector_approx(source, reference)
