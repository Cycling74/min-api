#pragma once

namespace c74 {
namespace min {
	
	
	class symbol {
	public:
		
		symbol(max::t_symbol* value) {
			s = value;
		}

		symbol& operator = (max::t_symbol* value) {
			s = value;
			return *this;
		}
		
		//		operator t_symbol*() const {
		//	return s;
		//}

		
	private:
		max::t_symbol* s;
	};
	
	
	
}} // namespace c74::min
