/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_min_attribute.h"

namespace c74 {
namespace min {

	template<class T, threadsafe threadsafety>
	template<typename ...ARGS>
	attribute<T,threadsafety>::attribute(object_base* an_owner, std::string a_name, T a_default_value, ARGS... args)
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
	, m_value			{ an_owner, a_name, static_cast<double>(a_default_value), false }
	{
		m_owner.attributes()[a_name] = this;

		m_datatype = k_sym_time;
		m_style = style::time;

		handle_arguments(args...);
		m_value.finalize();
		copy_range();
										
		set(to_atoms(a_default_value), false);
	}


	template<class T, threadsafe threadsafety>
	void attribute<T,threadsafety>::create(max::t_class* c, max::method getter, max::method setter, bool isjitclass) {
		if (m_style == style::time)
			class_time_addattr(c, m_name.c_str(), m_title.c_str(), 0);
		else if (isjitclass) {
			auto jit_attr = max::object_new_imp(max::gensym("jitter"),
												max::_jit_sym_jit_attr_offset,
												const_cast<void*>(static_cast<const void*>(m_name.c_str())),
												static_cast<max::t_symbol*>(datatype()),
												reinterpret_cast<void*>(flags(isjitclass)),
												reinterpret_cast<void*>(getter),
												reinterpret_cast<void*>(setter),
												nullptr, nullptr, nullptr);
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
//			auto jit_attr = max::jit_object_new(max::_jit_sym_jit_attr_offset_array,
//												m_name.c_str(),
//												(max::t_symbol*)datatype(),
//												0xFFFF,
//												flags(isjitclass),
//												getter,
//												setter,
//												(long)size_offset(),
//												0);
			auto jit_attr = max::object_new_imp(max::gensym("jitter"),
												max::_jit_sym_jit_attr_offset_array,
												const_cast<void*>(static_cast<const void*>(m_name.c_str())),
												static_cast<max::t_symbol*>(datatype()),
												reinterpret_cast<void*>(0xFFFF),
												reinterpret_cast<void*>(flags(isjitclass)),
												reinterpret_cast<void*>(getter),
												reinterpret_cast<void*>(setter),
												reinterpret_cast<void*>(size_offset()),
												nullptr);
			max::jit_class_addattr(c, jit_attr);
		}
		else {
			auto max_attr = max::attr_offset_array_new(m_name, datatype(), 0xFFFF, flags(isjitclass), getter, setter, size_offset(), 0);
			max::class_addattr(c, max_attr);
		}
	};


	// enum classes cannot be converted implicitly to the underlying type, so we do that explicitly here.
	template<class T, threadsafe threadsafety, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	std::string range_string_item(attribute<T,threadsafety>* attr, const T& item) {
		auto i = static_cast<int>(item);

		if (attr->get_enum_map().empty())
			return std::to_string(i);
		else
			return attr->get_enum_map()[i];
	}
	
	// all non-enum values can just pass through
	template<class T, threadsafe threadsafety, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
	T range_string_item(attribute<T,threadsafety>* attr, const T& item) {
		return item;
	}
	
	template<class T, threadsafe threadsafety>
	std::string attribute<T,threadsafety>::range_string() {
		std::stringstream ss;
		for (const auto& val : m_range)
			ss << "\"" << range_string_item<T,threadsafety>(this, val) << "\" ";
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
	template<class T, threadsafe threadsafety, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T,threadsafety>* attr) {
		for (auto i=0; i < attr->get_enum_map().size(); ++i)
			attr->range_ref().push_back(static_cast<T>(i));
	}


	// all non-enum attrs can just copy range normally
	template<class T, threadsafe threadsafety, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T,threadsafety>* attr) {
		for (const auto& a : attr->get_range_args())
			attr->range_ref().push_back(a);
	}

	template<class T, threadsafe threadsafety>
	void attribute<T,threadsafety>::copy_range() {
		range_copy_helper<T,threadsafety>(this);
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
