/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

#include "c74_min_attribute.h"

namespace c74 { namespace min {


	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions>
	template<typename... ARGS>
	attribute<T, threadsafety, limit_type, repetitions>::attribute(object_base* an_owner, std::string a_name, T a_default_value, ARGS... args)
	: attribute_base{*an_owner, a_name} {
		m_owner.attributes()[a_name] = this;

		if (is_same<T, bool>::value)
			m_datatype = k_sym_long;
		else if (is_same<T, int>::value)
			m_datatype = k_sym_long;
		else if (is_enum<T>::value)
			m_datatype = k_sym_long;
		else if (is_same<T, symbol>::value)
			m_datatype = k_sym_symbol;
		else if (is_same<T, float>::value)
			m_datatype = k_sym_float32;
		else
			m_datatype = k_sym_float64;

		if (is_same<T, bool>::value)
			m_style = style::onoff;
		else if (is_enum<T>::value)
			m_style = style::enum_index;
		else if (is_same<T, ui::color>::value)
			m_style = style::color;
		else if (a_name == "fontname")
			m_style = style::font;
		else
			m_style = style::none;

		handle_arguments(args...);
		copy_range();

		if (is_same<T,ui::color>::value && an_owner->is_ui_class()) {
			auto ui_op = dynamic_cast<ui_operator_base*>(an_owner);
			ui_op->add_color_attribute({a_name,this});
		}

		auto as = to_atoms(a_default_value);
		set(as, false, true);
	}


	template<>
	template<typename... ARGS>
	attribute<time_value>::attribute(object_base* an_owner, std::string a_name, time_value a_default_value, ARGS... args)
	: attribute_base{*an_owner, a_name}
	, m_value{an_owner, a_name, static_cast<double>(a_default_value)} {
		m_owner.attributes()[a_name] = this;

		m_datatype = k_sym_time;
		m_style    = style::time;

		handle_arguments(args...);
		copy_range();

		auto as = to_atoms(a_default_value);
		set(as, false, true);
	}


	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions>
	void attribute<T, threadsafety, limit_type, repetitions>::create(max::t_class* c, max::method getter, max::method setter, bool isjitclass) {
		if (m_style == style::time) {
			class_time_addattr(c, m_name.c_str(), m_title.c_str(), 0);
		}
		else if (isjitclass) {
			auto jit_attr = max::object_new_imp(max::gensym("jitter"), max::_jit_sym_jit_attr_offset,
				const_cast<void*>(static_cast<const void*>(m_name.c_str())), static_cast<max::t_symbol*>(datatype()),
				reinterpret_cast<void*>(flags(isjitclass)), reinterpret_cast<void*>(getter), reinterpret_cast<void*>(setter), nullptr,
				nullptr, nullptr);
			max::jit_class_addattr(c, jit_attr);
		}
		else {
			auto max_attr = max::attr_offset_new(m_name, datatype(), static_cast<long>(flags(isjitclass)), getter, setter, 0);
			max::class_addattr(c, max_attr);
		}
	};


	template<>
	void attribute<std::vector<double>>::create(max::t_class* c, max::method getter, max::method setter, bool isjitclass) {
		if (isjitclass) {
			auto jit_attr = max::object_new_imp(max::gensym("jitter"), max::_jit_sym_jit_attr_offset_array,
				const_cast<void*>(static_cast<const void*>(m_name.c_str())), static_cast<max::t_symbol*>(datatype()),
				reinterpret_cast<void*>(0xFFFF), reinterpret_cast<void*>(flags(isjitclass)), reinterpret_cast<void*>(getter),
				reinterpret_cast<void*>(setter), reinterpret_cast<void*>(size_offset()), nullptr);
			max::jit_class_addattr(c, jit_attr);
		}
		else {
			auto max_attr = max::attr_offset_array_new(
				m_name, datatype(), 0xFFFF, static_cast<long>(flags(isjitclass)), getter, setter, static_cast<long>(size_offset()), 0);
			max::class_addattr(c, max_attr);
		}
	};


	// enum classes cannot be converted implicitly to the underlying type, so we do that explicitly here.
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions,
		typename enable_if<std::is_enum<T>::value, int>::type = 0>
	std::string range_string_item(attribute<T, threadsafety, limit_type, repetitions>* attr, const T& item) {
		auto i = static_cast<int>(item);

		if (attr->get_enum_map().empty())
			return std::to_string(i);
		else
			return attr->get_enum_map()[i];
	}

	// vectors cannot be passed directly to stringstream
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions,
		typename enable_if<std::is_same<T, std::vector<number>>::value, int>::type = 0>
	std::string range_string_item(attribute<T, threadsafety, limit_type, repetitions>* attr, const T& item) {
		string str;
		for (const auto& i : item) {
			str += std::to_string(i);
			str += " ";
		}
	}

	// all non-enum non-vector values can just pass through
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions,
		typename enable_if<!std::is_enum<T>::value && !std::is_same<T, std::vector<number>>::value, int>::type = 0>
	T range_string_item(attribute<T, threadsafety, limit_type, repetitions>* attr, const T& item) {
		return item;
	}


	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions>
	std::string attribute<T, threadsafety, limit_type, repetitions>::range_string() {
		std::stringstream ss;
		for (const auto& val : m_range)
			ss << "\"" << range_string_item<T, threadsafety, limit_type, repetitions>(this, val) << "\" ";
		return ss.str();
	};


	template<>
	std::string attribute<std::vector<double>>::range_string() {
		if (m_range.empty())
			return "";

		// the range for this type is a low-bound and high-bound applied to all elements in the vector
		assert(m_range.size() == 2);

		std::stringstream ss;
		ss << m_range[0][0] << " " << m_range[1][0];
		return ss.str();
	};


	// enum attrs use the special enum map for range
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions, typename enable_if<is_enum<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T, threadsafety, limit_type, repetitions>* attr) {
		for (auto i = 0; i < attr->get_enum_map().size(); ++i)
			attr->range_ref().push_back(static_cast<T>(i));
	}


	// color attrs don't use range
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions,
		typename enable_if<is_color<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T, threadsafety, limit_type, repetitions>* attr) {
		// for (auto i=0; i < attr->get_enum_map().size(); ++i)
		//	attr->range_ref().push_back(static_cast<T>(i));
	}


	// most attrs can just copy range normally
	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions,
		typename enable_if<!is_enum<T>::value && !is_color<T>::value, int>::type = 0>
	void range_copy_helper(attribute<T, threadsafety, limit_type, repetitions>* attr) {
		for (const auto& a : attr->get_range_args())
			attr->range_ref().push_back(a);
	}


	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions>
	void attribute<T, threadsafety, limit_type, repetitions>::copy_range() {
		range_copy_helper<T, threadsafety, limit_type, repetitions>(this);
	};


	template<>
	void attribute<std::vector<double>>::copy_range() {
		if (!m_range.empty()) {
			// the range for this type is a low-bound and high-bound applied to all elements in the vector
			assert(m_range_args.size() == 2);

			m_range.resize(2);
			m_range[0][0] = m_range_args[0];
			m_range[1][0] = m_range_args[1];
		}
	};


	template<typename T, threadsafe threadsafety, template<typename> class limit_type, allow_repetitions repetitions>
	bool attribute<T, threadsafety, limit_type, repetitions>::compare_to_current_value(const atoms& args) {
		return (args[0] == m_value);
	}

	template<>
	bool attribute<number>::compare_to_current_value(const atoms& args) {
		return equivalent<number>(args[0], m_value);
	}

	template<>
	bool attribute<symbol>::compare_to_current_value(const atoms& args) {
		return (args[0] == m_value);
	}

	template<>
	bool attribute<vector<double>>::compare_to_current_value(const atoms& args) {
		if (args.size() == m_value.size()) {
			for (auto i=0; i<m_value.size(); ++i) {
				if (!equivalent<double>(args[i], m_value[i]))
					return false;
			}
			return true;
		}
		return false;
	}

	template<>
	bool attribute<ui::color>::compare_to_current_value(const atoms& args) {
		return equivalent<double>(args[0], m_value.red())
		&& equivalent<double>(args[1], m_value.green())
		&& equivalent<double>(args[2], m_value.blue())
		&& equivalent<double>(args[3], m_value.alpha());
	}


}}    // namespace c74::min
