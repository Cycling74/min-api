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
		virtual std::string range_string() = 0;
		
		/// Create the Max attribute and add it to the Max class
		virtual void create(max::t_class* c, max::method getter, max::method setter) = 0;
		
		/// calculate the offset of the size member as required for array/vector attributes
		size_t size_offset() {
			return (&m_size) - ((size_t*)&m_owner);
		}
		
	protected:
		object_base&	m_owner;
		symbol			m_name;
        symbol			m_title;
		symbol			m_datatype;
		function		m_setter;
		function		m_getter;
		bool			m_readonly = false;
		size_t			m_size;		/// size of array/vector if attr is array/vector
	};
	
	
	using title = std::string;
	using range = atoms;
	using readonly = bool;

	
	template <typename T>
	class attribute : public attribute_base {
	private:
		
		/// constructor utility: handle an argument defining an attribute's title / label
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, title>::value>::type
		assign_from_argument(const U& arg) noexcept {
			const_cast<symbol&>(m_title) = arg;
		}
		
		/// constructor utility: handle an argument defining a attribute's range
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, range>::value>::type
		assign_from_argument(const U& arg) noexcept {
			const_cast<U&>(m_range_args) = arg;
		}
		
		/// constructor utility: handle an argument defining a attribute's setter function
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, setter>::value>::type
		assign_from_argument(const U& arg) noexcept {
			const_cast<U&>(m_setter) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's getter function
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, getter>::value>::type
		assign_from_argument(const U& arg) noexcept {
			const_cast<U&>(m_getter) = arg;
		}
		
		/// constructor utility: handle an argument defining a attribute's readonly property
		template <typename U>
		constexpr typename std::enable_if<std::is_same<U, readonly>::value>::type
		assign_from_argument(const U& arg) noexcept {
			const_cast<U&>(m_readonly) = arg;
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
			copy_range();

			set(to_atoms(a_default_value), false);
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
					m_value = from_atoms<T>(m_setter(args));
				else
					m_value = from_atoms<T>(args);
			}
		}
		
		
		/// Create the Max attribute and add it to the Max class
		void create(max::t_class* c, max::method getter, max::method setter);
		
		
		operator atoms() const {
			return to_atoms(m_value);
		}
		
		
		operator T() const {
			return m_value;
		}
		

		std::string range_string();
		

		
	private:
		T				m_value;
		atoms			m_range_args;	// the range/enum as provided by the subclass
		std::vector<T>	m_range;		// the range/enum translated into the native datatype
		
		void copy_range();				// copy m_range_args to m_range
	};
	
	
	
	template<class T>
	void attribute<T>::create(max::t_class* c, max::method getter, max::method setter) {
		auto max_attr = c74::max::attr_offset_new(m_name, datatype(), 0, getter, setter, 0);
		c74::max::class_addattr(c, max_attr);
	};
	
	
	template<>
	void attribute<std::vector<double>>::create(max::t_class* c, max::method getter, max::method setter) {
		auto max_attr = c74::max::attr_offset_array_new(m_name, datatype(), 0xFFFF, 0, getter, setter, size_offset(), 0);
		c74::max::class_addattr(c, max_attr);
	};
	
	
	template<class T>
	std::string attribute<T>::range_string() {
		std::stringstream ss;
		for (const auto& val : m_range)
			ss << val << " ";
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
	
	
	template<class T>
	void attribute<T>::copy_range() {
		for (const auto& a : m_range_args)
			m_range.push_back(a);
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

	
	template<class T>
	max::t_max_err min_attr_getter(minwrap<T>* self, max::t_object* maxattr, long* ac, max::t_atom** av) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, k_sym_getname);
		auto& attr = self->min_object.attributes()[attr_name->s_name];
		atoms rvals = *attr;
		
		*ac = rvals.size();
		if (!(*av)) // otherwise use memory passed in
			*av = (max::t_atom*)max::sysmem_newptr(sizeof(max::t_atom) * *ac);
		for (auto i=0; i<*ac; ++i)
			(*av)[i] = rvals[i];
		
		return 0;
	}
	
	
	template<class T>
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, long ac, max::t_atom* av) {
		atom_reference args(ac,av);
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, k_sym_getname);
		auto attr = self->min_object.attributes()[attr_name->s_name];
		attr->set( atoms(args.begin(), args.end()), false );
		return 0;
	}

	
}} // namespace c74::min
