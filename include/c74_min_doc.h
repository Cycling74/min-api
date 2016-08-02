/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	using cstring = const char*;

	#define MIN_AUTHOR		static constexpr	char	class_author[]
	#define MIN_TAGS		static constexpr	cstring	class_tags[]
	#define MIN_DESCRIPTION static constexpr	char	class_description[]


	// The title and description types are just strings.
	// However, we have to define them unambiguously for the argument parsing in the attribute.

	class title : public std::string {
		using std::string::string; // inherit constructors
	};

	class description : public std::string {
		using std::string::string; // inherit constructors
	};



}} // namespace c74::min
