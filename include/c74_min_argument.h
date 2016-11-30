/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	using argument_function = std::function<void(const atom&)>;
	#define MIN_ARGUMENT_FUNCTION [this](const c74::min::atom& arg)

	class argument_base {
	public:
		argument_base(object_base* an_owner, const std::string& a_name, const description& a_description, bool required, const argument_function& a_function)
		: m_owner		{ an_owner }
		, m_name		{ a_name }
		, m_description	{ a_description }
		, m_required	{ required }
		, m_function	{ a_function }
		{
			m_owner->register_argument(this);
		}

		void operator ()(const atom& arg) {
			if (m_function)
				m_function(arg);
		}

		std::string description_string() const {
			return m_description;
		}

		symbol name() const {
			return m_name;
		}

		bool required() const {
			return m_required;
		}

		virtual std::string type() = 0;

	protected:
		object_base*		m_owner;
		symbol				m_name;
		description			m_description;
		bool				m_required;
		argument_function	m_function;
	};

	template<class T>
	class argument : public argument_base {
	public:
		argument(object_base* an_owner, const std::string& a_name, const description& a_description, const argument_function& a_function = {})
		: argument_base(an_owner, a_name, a_description, false, a_function)
		{}


		argument(object_base* an_owner, const std::string& a_name, const description& a_description, bool required, const argument_function& a_function = {})
		: argument_base(an_owner, a_name, a_description, required, a_function)
		{}


		std::string type() override {
			if		(is_same<T, bool>::value)		return "bool";
			else if (is_same<T, number>::value)		return "number";
			else if (is_same<T, float>::value)		return "float";
			else if (is_same<T, double>::value)		return "float";
			else if (is_same<T, int>::value)		return "int";
			else if (is_same<T, long>::value)		return "int";
			else if (is_same<T, symbol>::value)		return "symbol";
			else									return "";			// includes 'anything' type

		}
	};

}} // namespace c74::min
