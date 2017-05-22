/// @file
///	@ingroup 	minlib
/// @author		Timothy Place, Trond Lossius, Nils Peters
///	@copyright	Copyright (c) 2017, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {
namespace dataspace {

	class none : public dataspace_base {
	public:

		// the neutral unit is always a pass-through... compiler inlining should make it a noop
		class nothing {
			friend class dataspace_base;

			static inline number to_neutral(number input) {
				return input;
			}

			static inline number from_neutral(number input) {
				return input;
			}
		};

	};

}}}  // namespace c74::min::dataspace
