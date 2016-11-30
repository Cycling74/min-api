/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	// Finally, we define the min::object
	// Has to happen last because it includes code that does lazy initialization of the above class definitions
	// in the event that it hasn't occurred.

	/// We pass the class type as a template parameter to this class.
	/// This allows for [static polymorphism](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism).
	/// One benefits of this are leveraged when instantiating class instances directly instead of through the Max interface,
	/// such as when unit testing or embedding an object inside of another object.

	template<class min_class_type>
	class object : public object_base {
	public:
		object() {
			// The way objects are created for the Max environment requires that memory be allocated first
			// using object_alloc(), which is followed by the use of placement-new to contruct the C++ class.
			// When this occurs the m_maxobj member is already set prior to the constructor being run.
			// If there is no valid m_maxobj then that means this class was created outside of the Max environment.
			//
			// This could occur if a class uses another class directly or in the case of unit testing.
			// In such cases we need to do something reasonable so the our invariants can be held true.
			
			if (m_initializing) {							// we are being initialized externally via placement new
				;
			}
			else {											// we need to initialize ourselves
				if (!this_class_init) {						// if we aren't already in the process of initializing...
					std::string maxname = typeid(min_class_type).name();
					maxname += "_max";
					wrap_as_max_external<min_class_type> ( typeid(min_class_type).name(), maxname.c_str(), nullptr, static_cast<min_class_type*>(this) );
				}
				if (this_class) {
					m_maxobj = static_cast<max::t_object*>(max::object_alloc(this_class));
					postinitialize();
				}
			}			
		}
		
		virtual ~object() {}


		bool is_jitter_class() {
			return is_base_of<matrix_operator_base, min_class_type>::value;
		};


	protected:
		logger	cout { this, logger::type::message };
		logger	cerr { this, logger::type::error };
	};

}} // namespace c74::min

