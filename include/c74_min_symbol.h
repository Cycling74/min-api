#pragma once

namespace c74 {
namespace min {
	
	
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
