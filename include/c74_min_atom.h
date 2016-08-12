/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	

	class atom : public max::t_atom {
	public:
		
		/// Empty atom constructor
		atom() {
			this->a_type = c74::max::A_NOTHING;
			this->a_w.w_obj = nullptr;
		}
		
		/// Generic assigning constructor
		template<class T, typename enable_if< !std::is_enum<T>::value, int>::type = 0>
		atom(T initial_value) {
			*this = initial_value;
		}
		
		/// Enum assigning constructor
		template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
		atom(T initial_value) {
			*this = (long)initial_value;
		}
		
		
		atom& operator = (const max::t_atom& value) {
			this->a_type = value.a_type;
			this->a_w = value.a_w;
			return *this;
		}
		
		atom& operator = (const max::t_atom* init) {
			*this = *init;
			return *this;
		}
		
		atom& operator = (const long value) {
			atom_setlong(this, value);
			return *this;
		}

		atom& operator = (const int value) {
			atom_setlong(this, value);
			return *this;
		}

		atom& operator = (const bool value) {
			atom_setlong(this, value);
			return *this;
		}

		atom& operator = (const double value) {
			atom_setfloat(this, value);
			return *this;
		}
		
		atom& operator = (const max::t_symbol* value) {
			atom_setsym(this, value);
			return *this;
		}

		atom& operator = (const symbol value) {
			atom_setsym(this, value);
			return *this;
		}
		
		atom& operator = (const char* value) {
			atom_setsym(this, max::gensym(value));
			return *this;
		}
		
		atom& operator = (const std::string& value) {
			max::atom_setsym(this, max::gensym(value.c_str()));
			return *this;
		}

		atom& operator = (const max::t_object* value) {
			atom_setobj(this, (void*)value);
			return *this;
		}
		
		atom& operator = (const max::t_class* value) {
			atom_setobj(this, (void*)value);
			return *this;
		}
		
		atom& operator = (void* value) {
			atom_setobj(this, value);
			return *this;
		}

		
		/// Enum assigning constructor
		template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
		operator T() const {
			return (T)atom_getlong(this);
		}

		operator double() const {
			return atom_getfloat(this);
		}
		
		operator int() const {
			return int(atom_getlong(this));
		}

		operator long() const {
			return long(atom_getlong(this));
		}

		operator bool() const {
			return atom_getlong(this) != 0;
		}
		
		operator max::t_symbol*() const {
			return atom_getsym(this);
		}

		operator max::t_object*() const {
			return (max::t_object*)atom_getobj(this);
		}

		operator max::t_class*() const {
			return (max::t_class*)atom_getobj(this);
		}
		
		operator void*() const {
			return atom_getobj(this);
		}

		operator std::string() const {
			std::string s;
			
			switch (a_type) {
				case max::A_SEMI:
					s = ";";
					break;
				case max::A_COMMA:
					s = ",";
					break;
				case max::A_SYM:
					s = a_w.w_sym->s_name;
					break;
				case max::A_OBJ:
					if (a_w.w_obj)
						s = c74::max::object_classname(a_w.w_obj)->s_name;
					else
						s = "NULL";
					break;
				case max::A_LONG:
					s = std::to_string(a_w.w_long);
					break;
				case max::A_FLOAT:
					s = std::to_string(a_w.w_float);
					break;
				default:
					s = "?";
					break;
			}
			
			return s;
		}
		
		
		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, max::t_symbol* s) {
			return atom_getsym(&a) == s;
		}
		
		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, symbol s) {
			return atom_getsym(&a) == (max::t_symbol*)s;
		}

		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, const char* str) {
			return atom_getsym(&a) == max::gensym(str);
		}

		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, bool value) {
			return (atom_getlong(&a) != 0) == value;
		}
		
		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, int value) {
			return atom_getlong(&a) == value;
		}
		
		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, long value) {
			return atom_getlong(&a) == value;
		}
		
		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, double value) {
			return atom_getfloat(&a) == value;
		}

		/// Compare an atom against a value for equality.
		inline friend bool operator == (const max::t_atom& a, max::t_object* value) {
			return atom_getobj(&a) == value;
		}

		
	};
	

	// part of the symbol class but must be defined after atom is defined
	symbol& symbol::operator = (const atom& value) {
		s = value;
		return *this;
	}
	

	/// The atoms container is the standard means by which zero or more values are passed.
	/// It is implemented as a std::vector of the atom type, and thus atoms contained in an
	/// atoms container are 'owned' copies... not simply a reference to some externally owned atoms.

	// TODO: how to document inherited interface, e.g. size(), begin(), etc. ?
	
	using atoms = std::vector<atom>;


#ifdef __APPLE__
#pragma mark -
#pragma mark AtomRef
#endif


	/// The atom_reference type defines a container for atoms by reference, providing an interface
	/// that is interoperable with any of the classic standard library containers.
	///
	/// Typically you *do not use* the atom_reference type explicitly.
	/// It is rather intended as an intermediary between the atoms container type and
	/// old C-style functions in the Max API.
	/// As such it resembles some of the aims of the gsl::span type but serving a much more specialized purpose.
	///
	/// IMPORTANT: The size and order of members in this class are designed to make it a drop-in replace for
	/// the old C-style argc/argv pairs!  As such, no changes or additions should be made with regards to
	/// members, virtual methods, etc.
	
	class atom_reference {
	public:
		
		using size_type			= long;
		using value_type		= max::t_atom*;
		using iterator			= max::t_atom*;
		using const_iterator	= const max::t_atom*;
		
		iterator begin()				{ return m_av; }
		const_iterator begin() const	{ return m_av; }
		iterator end()					{ return m_av+m_ac; }
		const_iterator end() const		{ return m_av+m_ac; }
		
		size_type size() const			{ return m_ac; }
		bool empty() const				{ return size()==0; }
		
		// We don't own the array of atoms, so we cannot do these operations:
		// insert
		// erase
		// push_back
		// push_front
		// pop_front
		// pop_back
		
		// TODO: we could consider implementing the following,
		// but it is not clear we need them due to the limited roll of this type:
		// front()
		// back()
		// operator []
		// at()
		
		
		atom_reference(long argc, max::t_atom* argv)
		: m_ac { argc }
		, m_av { argv }
		{}
		
		atom_reference& operator = (const symbol& value) {
			m_ac = 1;
			atom_setsym(m_av, value);
			return *this;
		}
		
		operator atom() const {
			if (empty())
				throw std::out_of_range("atomref is empty");
			return atom(m_av);
		}
		
		operator atoms() const {
			atoms as(m_ac);
			
			for (auto i=0; i < m_ac; ++i)
				as[i] = m_av+i;
			return as;
		}
		
	private:
		long			m_ac;
		max::t_atom*	m_av;
	};
	
}} // namespace c74::min


#ifdef __APPLE__
#pragma mark -
#pragma mark Utilities
#endif


namespace std {
	
	/// overload of the std::to_string() function for the min::atoms type
	// it is perfectly legal to make this overload in the std namespace because it is overloaded on our user-defined type
	// as stated in section 17.6.4.2.1 of working draft version N4296 of the C++ Standard at
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4296.pdf
	
	string to_string(const c74::min::atoms& as) {
		long	textsize = 0;
		char*	text = nullptr;
		string	str;
		
		auto err = c74::max::atom_gettext((long)as.size(), &as[0], &textsize, &text, c74::max::OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE);
		if (!err)
			str = text;
		else
			c74::max::object_error(nullptr, "problem geting text from atoms");
		
		if (text)
			c74::max::sysmem_freeptr(text);
		
		return str;
	}
	
	
	/// overload of the std::to_string() function for the min::atom_reference type

	string to_string(const c74::min::atom_reference& ar) {
		c74::min::atoms as;
		for (const auto& ref: ar)
			as.push_back(ref);
		return to_string(as);
	}

}


/// Expose atom for use in std output streams.
template<class charT, class traits>
std::basic_ostream <charT, traits>& operator<< (std::basic_ostream <charT, traits>& stream, const c74::min::atom& a) {
	return stream << std::string(a);
}


namespace c74 {
namespace min {
		
	/// Copy values from any STL container to a vector of atoms
	/// @tparam	T			The type of the container
	/// @param	container	The container instance whose values will be copied
	/// @return				A vector of atoms
	
	template<class T, typename enable_if< !is_symbol<T>::value && !is_time_interval<T>::value && is_class<T>::value, int>::type = 0>
	atoms to_atoms(const T& container) {
		atoms	as(container.size());
		size_t	index = 0;
		
		for (const auto& item : container) {
			as[index] = item;
			++index;
		}
		return as;
	}


	/// Copy values from any simple type to a vector of atoms of size=1.
	/// @tparam	T	The type of the input value.
	/// @param	v	The value to be copied.
	/// @return		A vector of atoms

	template<class T, typename enable_if< is_symbol<T>::value || is_time_interval<T>::value || !is_class<T>::value, int>::type = 0>
	atoms to_atoms(const T& v) {
		atoms as {v};
		return as;
	}

	
	/// Copy values out from a vector of atoms to the desired container class
	/// @tparam	T	The type of the container
	/// @param	as	The vector atoms containing the desired data
	/// @return		The container of the values
	
	template<class T, typename enable_if< !is_symbol<T>::value && !is_time_interval<T>::value && is_class<T>::value, int>::type = 0>
	T from_atoms(const atoms& as) {
		T container;
		
		container.reserve(as.size());
		for (const auto& a : as)
			container.push_back(a);
		return container;
	}
	
	
	/// Copy a value out from a vector of atoms to the desired type
	/// @tparam	T	The type of the destination variable
	/// @param	as	The vector atoms containing the desired data
	/// @return		The value
	
	template<class T, typename enable_if< !std::is_enum<T>::value && (is_symbol<T>::value || is_time_interval<T>::value || !is_class<T>::value), int>::type = 0>
	T from_atoms(const atoms& as) {
		return (T)as[0];
	}
	
	template<class T, typename enable_if< std::is_enum<T>::value, int>::type = 0>
	T from_atoms(const atoms& as) {
		return (T)(long)as[0];
	}

	
}}
