/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	// Type definition for what the legacy C Max SDK uses to represent an outlet.

	using t_max_inlet = void*;


	// Represents any type of inlet.
	// Used internally to allow heterogenous containers of inlets.

	class inlet_base : public port {
		friend void object_base::create_inlets();

	public:
		inlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: port { an_owner, a_description, a_type}
		{}

	private:
		t_max_inlet m_instance { nullptr };
	};


	/// The actual inlet class is a templated type.
	/// This is done for consistency of coding style with outlet<>, message<>, attribute<>, etc.
	/// At the moment there is no actual function for the template parameter.
	/// This enum thus represents a placeholder type that can be used in the future if need arises.

	enum class inlet_placeholder {
		none	///< No flags, functions, or other alterations to the inlet class.
	};


	/// An inlet.
	/// @tparam	Unused. You should supply no arguments. For example, `inlet<>`.

	template<inlet_placeholder inlet_placeholder_type = inlet_placeholder::none>
	class inlet : public inlet_base {
	public:

		/// Create an Inlet.
		/// @param	an_owner		The owning object for the inlet. Typically you will pass `this`.
		/// @param	a_description	Description of the inlet for documentation and UI assistance.
		/// @param	a_type			Optional string to create a type-checked inlet.

		inlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: inlet_base { an_owner, a_description, a_type}
		{
			m_owner->inlets().push_back(this);
		}
	};

}} // namespace c74::min
