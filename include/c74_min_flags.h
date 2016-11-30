/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	enum class documentation_flags : int {
		none,
		do_not_generate
	};

	enum class behavior_flags : int {
		none,
		nobox	///< cannot create in a max box (i.e. it is an internal-use-only class)
	};

	
	class flags {
	public:
		explicit constexpr flags(documentation_flags doc)
		: m_documentation {doc}
		{}

		explicit constexpr flags(behavior_flags behavior)
		: m_behavior {behavior}
		{}

		explicit constexpr flags(behavior_flags behavior, documentation_flags doc)
		: m_documentation {doc}
		, m_behavior {behavior}
		{}

		constexpr operator documentation_flags() const {
			return m_documentation;
		}

		constexpr operator behavior_flags() const {
			return m_behavior;
		}

	private:
		documentation_flags m_documentation { documentation_flags::none };
		behavior_flags		m_behavior		{ behavior_flags::none };
	};



	#define MIN_FLAGS const flags class_flags

	template<typename min_class_type>
	struct has_class_flags {
		template<class,class> class checker;

		template<typename C>
		static std::true_type test(checker<C, decltype(&C::class_flags)>*);

		template<typename C>
		static std::false_type test(...);

		typedef decltype(test<min_class_type>(nullptr)) type;
		static const bool value = is_same<std::true_type, decltype(test<min_class_type>(nullptr))>::value;
	};


	template<class min_class_type, class T>
	typename enable_if< has_class_flags<min_class_type>::value>::type
	class_get_flags(const min_class_type& instance, T& returned_flags) {
		returned_flags = instance.class_flags;
	}

	template<class min_class_type, class T>
	typename enable_if< !has_class_flags<min_class_type>::value>::type
	class_get_flags(const min_class_type& instance, T& returned_flags) {
		returned_flags = T::none;
	}

}} // namespace c74::min
