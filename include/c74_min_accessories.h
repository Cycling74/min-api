/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <random>

namespace c74 {
namespace min {
namespace math {
	
	/// code from http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
	/// see also http://en.cppreference.com/w/cpp/numeric/random
	
	double random(double min, double max) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(min, max);
		return dis(gen);
	}

	
}}} // namespace c74::min::math
