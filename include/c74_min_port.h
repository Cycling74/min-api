/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	/// A port represents a input or an output from an object.
	/// It is the base class for both #inlet and #outlet and is not intended to be created directly.
	///
	/// @seealso #inlet
	/// @seealso #outlet

	class port {
	protected:
		port(object_base* an_owner, const std::string& a_description, const std::string& a_type)
		: m_owner(an_owner)
		, m_description(a_description)
		, m_type(a_type)
		{}

	public:

		/// Determine if an audio signal is currently connected to this port.
		///	@return		True if a signal is connected. Otherwise false.

		bool has_signal_connection() {
			return m_signal_connection;
		}


		/// Determine the type of the port.
		/// Most inlets and outlets are generic and thus the type is an empty string.
		/// Notable exceptions are "signal" for audio and "dictionary" for dictionaries.
		/// @return		A string with the type of the port, if a type has been given to the port.

		const std::string& type() {
			return m_type;
		}


		/// Get the description of the port for either documentation generation
		/// or displaying assistance info in the UI.
		/// @return		A string with the description of the port.

		const std::string& description() {
			return m_description;
		}

	protected:
		object_base*	m_owner;
		std::string		m_description;
		std::string		m_type;
		bool			m_signal_connection { false };


		// This is called by Max's "dspsetup" method to update our audio signal connection state.

		void update_signal_connection(bool new_signal_connection_status) {
			m_signal_connection = new_signal_connection_status;
		}
	};


}} // namespace c74::min
