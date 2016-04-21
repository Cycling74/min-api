#pragma once

namespace c74 {
namespace min {
	
	
	class atom : public max::t_atom {
	public:
		
		atom(const max::t_atom& init) {
			*this = init;
		}
		
		atom(const long value) {
			atom_setlong(this, value);
		}
		
		atom(const double value) {
			atom_setfloat(this, value);
		}
		
		
		atom& operator = (max::t_atom value) {
			this->a_type = value.a_type;
			this->a_w = value.a_w;
			return *this;
		}
		
		atom& operator = (const long value) {
			atom_setlong(this, value);
			return *this;
		}

		atom& operator = (const double value) {
			atom_setfloat(this, value);
			return *this;
		}
		
		
		
		operator double() const {
			return atom_getfloat(this);
		}
		
		operator long() const {
			return atom_getfloat(this);
		}
		
		operator symbol() const {
			return symbol(atom_getsym(this));
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
						s = object_classname(a_w.w_obj)->s_name;
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
		
		
		/** Compare a symbols against a c-string for equality. */
		inline friend bool operator == (const max::t_atom& a, max::t_symbol* s)
		{
			return atom_getsym(&a) == s;
		}
		
	};
	
	
	using atoms = std::vector<atom>;
	
	
	// TODO: move the atoms on return instead of copy
	atoms atoms_from_acav(long ac, max::t_atom* av) {
		atoms as;
		
		for (int i=0; i<ac; ++i)
			as.push_back(av[i]);
		return as;
	}	
	
}} // namespace c74::min
