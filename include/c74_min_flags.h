/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	enum class documentation_flags {
		none,
		do_not_generate
	};

	
	class flags {
	public:
		constexpr flags(documentation_flags docflags)
		: m_documentation_flags {docflags}
		{}

		constexpr operator documentation_flags() const {
			return m_documentation_flags;
		}

	private:
		documentation_flags m_documentation_flags {documentation_flags::none};
	};



	#define MIN_FLAGS static const constexpr flags class_flags

	template<typename min_class_type>
	struct has_class_flags {
		template<class,class> class checker;

		template<typename C>
		static std::true_type test(checker<C, decltype(&C::class_flags)>);

		template<typename C>
		static std::false_type test(...);

		typedef decltype(test<min_class_type>(nullptr)) type;
		static const bool value = is_same<std::true_type, decltype(test<min_class_type>(nullptr))>::value;
	};

}} // namespace c74::min
