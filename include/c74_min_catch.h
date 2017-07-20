/// @file
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2017, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "catch.hpp"
#include "c74_min.h"


template<typename T>
bool require_vector_approx(T source, T reference) {
	REQUIRE( source.size() == reference.size() );
	if (source.size() != reference.size())
		return false;

	for (auto i=0; i<source.size(); ++i) {
		REQUIRE( source[i] == Approx(reference[i]) );
		if (source[i] != Approx(reference[i]))
			return false;
	}
	return true;
}


#define REQUIRE_VECTOR_APPROX( source, reference ) require_vector_approx(source, reference)
