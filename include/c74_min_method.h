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
		method(object_base* an_owner, std::string a_name, function a_function)
		: owner(an_owner)
		, function(a_function)
		{
			if (a_name == "integer")
				a_name = "int";
			else if (a_name == "number")
				a_name = "float";
			else if (a_name == "dsp64" || a_name == "dblclick" || a_name == "edclose" || a_name == "okclose" || a_name == "patchlineupdate")
				type = max::A_CANT;
			owner->methods()[a_name] = this;
		}
		
		atoms operator ()(atoms args = {}) {
			return function(args);
		}
		
		atoms operator ()(atom arg) {
			return function({arg});
		}
		
		//private:
		object_base*	owner;
		long			type = max::A_GIMME;
		function		function;
	};
	
	
	atoms object_base::try_call(const std::string& name, const atoms& args) {
		auto meth = m_methods.find(name);
		if (meth != m_methods.end())
			return (*meth->second)(args);
		return {};
	}

}} // namespace c74::min
