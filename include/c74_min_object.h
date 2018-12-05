/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74 { namespace min {


	/// The base class for all first-class objects that are to be exposed in the Max environment.
	///
	/// We pass the class type as a template parameter to this class.
	/// This allows for [static polymorphism](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism).
	/// One benefits of this are leveraged when instantiating class instances directly instead of through the Max interface,
	/// such as when unit testing or embedding an object inside of another object.
	///
	/// @tparam min_class_type	The name of your class that is extenting min::object.
	/// @tparam threadsafety	The default threadsafety assumption for all messages and attributes in this class.

	template<class min_class_type, threadsafe threadsafety = threadsafe::no>
	class object : public object_base {
	public:
		/// Constructor.

		object() {

			// The way objects are created for the Max environment requires that memory be allocated first
			// using object_alloc() or jit_object_alloc(), which is followed by the use of placement-new to contruct the C++ class.
			//
			// When this occurs the m_maxobj member is already set prior to the constructor being run.
			// If there is no valid m_maxobj then that means this class was created outside of the Max environment.
			//
			// This could occur if a class uses another class directly or in the case of unit testing.
			// In such cases we need to do something reasonable so that our invariants can be held true
		}

		/// Destructor.

		virtual ~object() {}


		bool is_jitter_class() override {
			return is_base_of<matrix_operator_base, min_class_type>::value;
		};

		bool is_ui_class() override {
			return is_base_of<ui_operator_base, min_class_type>::value;
		}

		bool is_assumed_threadsafe() override {
			return threadsafety == threadsafe::yes;
		}


	protected:
		logger cout{this, logger::type::message};
		logger cerr{this, logger::type::error};
	};

}}    // namespace c74::min
