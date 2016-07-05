/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <sstream>

namespace c74 {
namespace min {
	
	
	class inlet;
	class outlet;
	class method;
	class attribute_base;
	
	template <typename T>
	class attribute;

	class sample_operator_base;
	class perform_operator_base;
	class matrix_operator_base;
	class gl_operator_base;
	

	
	
	
	using setter = function;
	using getter = std::function<atoms()>;;

	
	class attribute_base {
	public:
		attribute_base(object_base& an_owner, std::string a_name)
		: m_owner(an_owner)
		, m_name(a_name)
        , m_title(a_name)
		{}
		
		/// set the value of the attribute
		virtual attribute_base& operator = (atoms args) = 0;
		
		/// set the value of the attribute
		virtual void set(const atoms& args, bool notify = true) = 0;
		
		/// get the value of the attribute
		virtual operator atoms() const = 0;

		/// fetch the name of the datatype
		symbol datatype() {
			return m_datatype;
		}
		
		/// fetch the title/label as a string
		const char* label_string() {
			return m_title;
		}
		
		/// fetch the range in string format, values separated by spaces
		virtual const char* range_string() = 0;
		
	protected:
		object_base&	m_owner;
		symbol			m_name;
        symbol			m_title;
		symbol			m_datatype;
		function		m_setter;
		function		m_getter;
	};
	
	
	using title = std::string;
	using range = atoms;

	
	template <typename T>
	class attribute : public attribute_base {
	private:
		
		/// constructor utility: handle an argument defining an attribute's digest / label
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, title>::value>::type
		assign_from_argument(const U& arg) noexcept {
			m_title = arg;
		}
		
		/// constructor utility: handle an argument defining a parameter's range
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, range>::value>::type
		assign_from_argument(const U& arg) noexcept {
			for (const auto& a : arg)
				m_range.push_back(a);
		}
		
		/// constructor utility: handle an argument defining a parameter's setter function
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, setter>::value>::type
		assign_from_argument(const U& arg) noexcept {
			m_setter = arg;
		}

		/// constructor utility: handle an argument defining a parameter's setter function
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, getter>::value>::type
		assign_from_argument(const U& arg) noexcept {
			m_getter = arg;
		}

		/// constructor utility: empty argument handling (required for recursive variadic templates)
		constexpr void handle_arguments() noexcept {
			;
		}
		
		/// constructor utility: handle N arguments of any type by recursively working through them
		///	and matching them to the type-matched routine above.
		template <typename FIRST_ARG, typename ...REMAINING_ARGS>
		constexpr void handle_arguments(FIRST_ARG const& first, REMAINING_ARGS const& ...args) noexcept {
			assign_from_argument(first);
			if (sizeof...(args))
				handle_arguments(args...);		// recurse
		}
		
		
	public:
		/// Constructor
		/// @param an_owner			The instance pointer for the owning C++ class, typically you will pass 'this'
		/// @param a_name			A string specifying the name of the attribute when dynamically addressed or inspected.
		/// @param a_default_value	The default value of the attribute, which will be set when the instance is created.
		/// @param ...args			N arguments specifying optional properties of an attribute such as setter, label, style, etc.
		template <typename...ARGS>
		attribute(object_base* an_owner, std::string a_name, T a_default_value, ARGS...args)
		: attribute_base(*an_owner, a_name) {
			m_owner.attributes()[a_name] = this;
			
			if (std::is_same<T, bool>::value)				m_datatype = k_sym_long;
			else if (std::is_same<T, int>::value)			m_datatype = k_sym_long;
			else if (std::is_same<T, symbol>::value)		m_datatype = k_sym_symbol;
			else if (std::is_same<T, float>::value)			m_datatype = k_sym_float32;
			else /* (std::is_same<T, double>::value) */		m_datatype = k_sym_float64;

			handle_arguments(args...);
			
			atoms a { a_default_value };
			set(a, false);
		}
		
		
		/// Set the attribute value using the native type of the attribute.
		attribute& operator = (const T arg) {
			atoms as = { atom(arg) };
			*this = as;
			return *this;
		}
		
		/// Set the attribute value using atoms.
		attribute& operator = (atoms args) {
			set(args);
			return *this;
		}
		
		/// Set the attribute value
		void set(const atoms& args, bool notify = true) {
			if (notify && this_class)
				max::object_attr_setvalueof(m_owner, m_name, args.size(), (max::t_atom*)&args[0]);
			else {
				if (m_setter)
					m_value = m_setter(args)[0];
				else
					m_value = args[0];
			}
		}
		
		
		
		operator atoms() const {
			atom a = m_value;
			atoms as = { a };
			return as;
		}
		
		
		operator T() const {
			return m_value;
		}
		
		
		const char* range_string() {
			std::stringstream ss;
			for (const auto& val : m_range)
				ss << val << " ";
			return ss.str().c_str();
		};

		
	private:
		T				m_value;
		std::vector<T>	m_range;
	};
	
	
	static max::t_symbol* ps_getname = max::gensym("getname");
	

	
	template<class T>
	max::t_max_err min_attr_getter(minwrap<T>* self, max::t_object* maxattr, long* ac, max::t_atom** av) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, ps_getname);
		auto& attr = self->obj.attributes()[attr_name->s_name];
		atoms rvals = *attr;
		
		*ac = rvals.size();
		if (!(*av)) // otherwise use memory passed in
			*av = (max::t_atom*)max::sysmem_newptr(sizeof(max::t_atom) * *ac);
		for (auto i=0; i<*ac; ++i)
			(*av)[i] = rvals[i];
		
		return 0;
	}
	
	
	template<class T>
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, atom_reference args) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, ps_getname);
		auto attr = self->obj.attributes()[attr_name->s_name];
		attr->set( atoms(args.begin(), args.end()), false );
		return 0;
	}

	
}} // namespace c74::min
