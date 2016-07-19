/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	using function = std::function<atoms(const atoms&)>;
	#define MIN_FUNCTION [this](const c74::min::atoms& args) -> c74::min::atoms
	
	
	class method {
	public:
		method(object_base* an_owner, const std::string& a_name, const function& a_function)
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		{
			std::string name;
			
			if (a_name == "integer")
				name = "int";
			else if (a_name == "number")
				name = "float";
			else if (a_name == "dsp64" || a_name == "dblclick" || a_name == "edclose" || a_name == "okclose" || a_name == "patchlineupdate")
				m_type = max::A_CANT;
			m_owner->methods()[name] = this;
		}
		
		atoms operator ()(atoms args = {}) {
			return m_function(args);
		}
		
		atoms operator ()(atom arg) {
			return m_function({arg});
		}
		
		long type() {
			return m_type;
		}
		
	private:
		object_base*	m_owner;
		function		m_function;
		long			m_type { max::A_GIMME };
	};
	
	
	atoms object_base::try_call(const std::string& name, const atoms& args) {
		auto meth = m_methods.find(name);
		if (meth != m_methods.end())
			return (*meth->second)(args);
		return {};
	}

}} // namespace c74::min
