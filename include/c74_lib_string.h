/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {


	
	namespace string_utility {
		using std::string;
		using std::vector;


		/// Trim leading and trailing whitespace from a string
		/// @param	s	The string to trim
		/// @return		The trimmed string

		inline string trim(string& s) {
			if (!s.empty()) {
				size_t first = s.find_first_not_of(' ');
				size_t last = s.find_last_not_of(' ');
				return s.substr(first, (last - first + 1));
			}
			else
				return s;
		}


		/// Split a string into a vector of substrings on a specified delimiter
		/// @param	s		The string to split
		/// @param	delim	The delimiter on which to split the string
		/// @return			A vector of substrings

		inline vector<string> split(const string &s, char delim) {
			vector<string>		substrings;
			string				substring;
			std::stringstream	ss(s);

			while (getline(ss, substring, delim))
				substrings.push_back(substring);
			return substrings;
		}


		inline string join(const vector<string>& input, char glue = ' ') {
			string output;

			for (const auto& str : input) {
				output += str;
				output += " ";
			}
			trim(output);
			return output;
		}

	} // namespace string_utility
	
	

}} // namespace c74::min
