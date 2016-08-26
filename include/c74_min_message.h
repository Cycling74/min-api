/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	using function = std::function<atoms(const atoms&)>;
	#define MIN_FUNCTION [this](const c74::min::atoms& args) -> c74::min::atoms
	
	
	class message {
	public:
		message(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {})
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		, m_description	{ a_description }
		{
			assert(m_function != nullptr); // could happen if a function is passed as the arg but that fn hasn't initialized yet

			std::string name = a_name;
			
			if (name == "integer")
				name = "int";
			else if (name == "number")
				name = "float";
			else if (   a_name == "dblclick"
					 || a_name == "dsp64"
					 || a_name == "dspsetup"
					 || a_name == "edclose"
					 || a_name == "jitclass_setup"
					 || a_name == "maxclass_setup"
					 || a_name == "maxob_setup"
					 || a_name == "mop_setup"
					 || a_name == "notify"
					 || a_name == "okclose"
					 || a_name == "patchlineupdate"
					 || a_name == "savestate"
					 || a_name == "setup"
			) {
				m_type = max::A_CANT;
			}

			m_name = name;
			m_owner->messages()[name] = this;
		}

		message(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message(an_owner, a_name, a_function, a_description)
		{}

		
		atoms operator ()(atoms args = {}) {
			return m_function(args);
		}
		
		atoms operator ()(atom arg) {
			return m_function({arg});
		}
		
		long type() const {
			return m_type;
		}

		std::string description_string() const {
			return m_description;
		}

		symbol name() const {
			return m_name;
		}
		
	private:
		object_base*	m_owner;
		function		m_function;
		long			m_type { max::A_GIMME };
		symbol			m_name;
		description		m_description;
	};
	
	
	atoms object_base::try_call(const std::string& name, const atoms& args) {
		auto found_message = m_messages.find(name);
		if (found_message != m_messages.end())
			return (*found_message->second)(args);
		return {};
	}

}} // namespace c74::min
