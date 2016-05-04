#pragma once

namespace c74 {
namespace min {
	
	
	class symbol {
	public:
		
		symbol() {
			s = max::gensym("");
		}
		
		symbol(max::t_symbol* value) {
			s = value;
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
	
	
	
}} // namespace c74::min
