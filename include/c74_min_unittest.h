/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "c74_min.h"

namespace c74 {
	namespace min {

		template<class min_class_type>
		class test_wrapper {
		public:
			test_wrapper() {
				m_minwrap_obj = wrapper_new<min_class_type>(symbol("dummy"), 0, nullptr);
			}

			~test_wrapper() {
				max::object_free(m_minwrap_obj);
			}

			operator min_class_type&() {
				return m_minwrap_obj->min_object;
			}

		private:
			minwrap<min_class_type>* m_minwrap_obj = nullptr;
		};

	} // namespace min


	namespace max {

		///	A vector of atoms.
		using t_atom_vector = min::atoms;

		/// A sequence of atom vectors.
		/// Used to log inlet and outlet activity in the mock environment.
		/// We can use logging of inlet and outlet sequences for BDD.
		/// We can also do more traditional state-based testing.
		/// Or mix-n-match as we see fit...
		///
		/// @remark		should sequences have time-stamps?
		typedef std::vector<t_atom_vector>	t_sequence;

		extern "C" t_sequence* object_getoutput(void *o, int outletnum);
	
	} // namespace max

} // namespace c74
