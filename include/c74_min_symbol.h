/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class atom;
	
	
	
	class symbol {
	public:
		
		symbol(bool unique = false) {
			if (unique)
				s = max::symbol_unique();
			else
				s = max::gensym("");
		}
		
		symbol(max::t_symbol* value) {
			s = value;
		}
		
		symbol(const char* value) {
			s = max::gensym(value);
		}
		
		symbol(const std::string& value) {
			s = max::gensym(value.c_str());
		}
		
		symbol(const atom& value); // defined in c74_min_atom.h
		

		symbol& operator = (max::t_symbol* value) {
			s = value;
			return *this;
		}

		symbol& operator = (const char* value) {
			s = max::gensym(value);
			return *this;
		}

		symbol& operator = (const std::string& value) {
			s = max::gensym(value.c_str());
			return *this;
		}

		symbol& operator = (const atom& value); // defined in c74_min_atom.h

		
		
		friend bool operator == (const symbol& lhs, const symbol& rhs) {
			return lhs.s == rhs.s;
		}

		friend bool operator == (const symbol& lhs, const char* rhs) {
			return lhs.s == max::gensym(rhs);
		}

		
		
		operator max::t_symbol*() const {
			return s;
		}
		
		operator const char*() const {
			return s->s_name;
		}
		
		operator max::t_dictionary*() const {
			return s->s_thing;
		}

		
		
		
	private:
		max::t_symbol* s;
	};
	
	
	/// Expose symbol for use in std output streams.
	template <class charT, class traits>
	std::basic_ostream <charT, traits>& operator<< (std::basic_ostream <charT, traits>& stream, const min::symbol& s) {
		return stream << (const char*)s;
	}
	
	
	#ifdef __APPLE__
	#pragma mark -
	#pragma mark Cache of Pre-Defined Symbols
	#endif
	
	static const symbol k_sym__empty("");				/// The special empty symbol which contains no chars at all.
	static const symbol k_sym__pound_d("#D");			/// The special "#D" symbol used for accessing an object's dictionary in the patcher.
	static const symbol k_sym_float32("float32");		/// The symbol "float32".
	static const symbol k_sym_float64("float64");		/// The symbol "float64".
	static const symbol k_sym_getmatrix("getmatrix");	/// The symbol "getmatrix".
	static const symbol k_sym_long("long");				/// The symbol "long".
	static const symbol k_sym_modified("modified");		/// The symbol "modified".
	static const symbol k_sym_symbol("symbol");			/// The symbol "symbol".
	static const symbol k_sym_list("list");				/// The symbol "list".
	static const symbol k_sym_getname("getname");		/// The symbol "getname".

	
}} // namespace c74::min
