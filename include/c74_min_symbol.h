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

		symbol& operator = (max::t_symbol* value) {
			s = value;
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
		
		
	private:
		max::t_symbol* s;
	};
	
	
	
	static const symbol k_sym__empty("");
	
	
}} // namespace c74::min
