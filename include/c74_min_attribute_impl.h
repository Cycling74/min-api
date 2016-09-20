/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_min_attribute.h"

namespace c74 {
namespace min {

	template<class T>
	template<typename ...ARGS>
	attribute<T>::attribute(object_base* an_owner, std::string a_name, T a_default_value, ARGS... args)
	: attribute_base { *an_owner, a_name }
	{
		m_owner.attributes()[a_name] = this;

		if (is_same<T, bool>::value)			m_datatype = k_sym_long;
		else if (is_same<T, int>::value)		m_datatype = k_sym_long;
		else if (is_enum<T>::value)				m_datatype = k_sym_long;
		else if (is_same<T, symbol>::value)		m_datatype = k_sym_symbol;
		else if (is_same<T, float>::value)		m_datatype = k_sym_float32;
		else									m_datatype = k_sym_float64;

		if (is_same<T, bool>::value)			m_style = style::onoff;
		else									m_style = style::none;

		handle_arguments(args...);
		copy_range();

		set(to_atoms(a_default_value), false);
	}

	template<>
	template<typename ...ARGS>
	attribute<time_value>::attribute(object_base* an_owner, std::string a_name, time_value a_default_value, ARGS... args)
	: attribute_base	{ *an_owner, a_name }
	, m_value			{ an_owner, a_name, double(a_default_value) }
	{
		m_owner.attributes()[a_name] = this;

		m_datatype = k_sym_time;
		m_style = style::time;

		handle_arguments(args...);
		copy_range();
										
		set(to_atoms(a_default_value), false);
	}


	
	template<class T>
	void attribute<T>::create(max::t_class* c, max::method getter, max::method setter, bool isjitclass) {
		if (m_style == style::time)
			class_time_addattr(c, m_name.c_str(), m_title.c_str(), 0);
		else if (isjitclass) {
			auto jit_attr = max::jit_object_new(max::_jit_sym_jit_attr_offset, m_name, datatype(), flags(isjitclass), getter, setter, 0);
			max::jit_class_addattr(c, jit_attr);
		}
		else {
			auto max_attr = max::attr_offset_new(m_name, datatype(), flags(isjitclass), getter, setter, 0);
			max::class_addattr(c, max_attr);
		}
	};
	
	
	template<>
	void attribute<std::vector<double>>::create(max::t_class* c, max::method getter, max::method setter, bool isjitclass) {
		if (isjitclass) {
			auto jit_attr = max::jit_object_new(max::_jit_sym_jit_attr_offset_array, m_name.c_str(), (max::t_symbol*)datatype(), 0xFFFF, flags(isjitclass), getter, setter, (long)size_offset(), 0);
			max::jit_class_addattr(c, jit_attr);
		}
		else {
			auto max_attr = max::attr_offset_array_new(m_name, datatype(), 0xFFFF, flags(isjitclass), getter, setter, (long)size_offset(), 0);
			max::class_addattr(c, max_attr);
		}
	};


	// enum classes cannot be converted implicitly to the underlying type, so we do that explicitly here.
	template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	std::string range_string_item(attribute<T>* attr, const T& item) {
		if (attr->get_enum_map().empty())
			return std::to_string((int)item);
		else
			return attr->get_enum_map()[(int)item];
	}
	
	// all non-enum values can just pass through
	template<class T, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
	T range_string_item(attribute<T>* attr, const T& item) {
		return item;
	}
	
	template<class T>
	std::string attribute<T>::range_string() {
		std::stringstream ss;
		for (const auto& val : m_range)
			ss << "\"" << range_string_item<T>(this, val) << "\" ";
		return ss.str();
	};


	template<>
	std::string attribute<std::vector<double>>::range_string() {
		if (m_range.empty())
			return "";
		
		// the range for this type is a low-bound and high-bound applied to all elements in the vector
		assert( m_range.size() == 2);
	
		std::stringstream ss;
		ss << m_range[0][0] << " " << m_range[1][0];
		return ss.str();
	};



	// enum attrs use the special enum map for range
	template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T>* attr) {
		for (auto i=0; i < attr->get_enum_map().size(); ++i)
			attr->range_ref().push_back((T)i);
	}


	// all non-enum attrs can just copy range normally
	template<class T, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T>* attr) {
		for (const auto& a : attr->get_range_args())
			attr->range_ref().push_back(a);
	}

	template<class T>
	void attribute<T>::copy_range() {
		range_copy_helper<T>(this);
	};
		
	template<>
	void attribute<std::vector<double>>::copy_range() {
		if (!m_range.empty()) {
			// the range for this type is a low-bound and high-bound applied to all elements in the vector
			assert( m_range_args.size() == 2);
			
			m_range.resize(2);
			m_range[0][0] = m_range_args[0];
			m_range[1][0] = m_range_args[1];
		}
	};

/*
	template<class T, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
	T range_apply_helper(const T& value) {
		// TODO: implement for the generic case, if the object author has requested it
		return value;
	}

	template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	T range_apply_helper(const T& value) {
		// TODO: implement for enums
		return value;
	}


	template<class T>
	T attribute<T>::range_apply(const T& value) {
		// TODO: implement for the generic case, if the object author has requested it
		return range_apply_helper(value);
		// TODO: above should use perfect forwarding?
	}
*/

}} // namespace c74::min
