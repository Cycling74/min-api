#pragma once

namespace c74 {
namespace min {
	
	class dict;
	
	class atom : public max::t_atom {
	public:
		
		atom() {
			this->a_type = c74::max::A_NOTHING;
			this->a_w.w_obj = nullptr;
		}
		
		atom(const max::t_atom& init) {
			*this = init;
		}
		
		atom(const long value) {
			atom_setlong(this, value);
		}
		
		atom(const bool value) {
			atom_setlong(this, value);
		}

		atom(const double value) {
			atom_setfloat(this, value);
		}
		
		atom(const symbol value) {
			max::atom_setsym(this, value);
		}

		atom(const max::t_symbol* value) {
			max::atom_setsym(this, value);
		}
		
		atom(const max::t_dictionary* value) {
			max::atom_setobj(this, (void*)value);
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
		
		
		operator double() const {
			return atom_getfloat(this);
		}
		
		operator long() const {
			return atom_getlong(this);
		}

		operator bool() const {
			return atom_getlong(this);
		}

		operator symbol() const {
			return symbol(atom_getsym(this));
		}
		
		operator max::t_symbol*() const {
			return atom_getsym(this);
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
		inline friend bool operator == (const max::t_atom& a, max::t_symbol* s) {
			return atom_getsym(&a) == s;
		}
		
		
		inline friend bool operator == (const max::t_atom& a, bool value) {
			return atom_getlong(&a) == value;
		}
		
		
		
	};
	
	
	using atoms = std::vector<atom>;
	
	std::string atoms_to_string(const atoms& as) {
		long		textsize = 0;
		char*		text = nullptr;
		std::string	str;
		
		auto err = max::atom_gettext(as.size(), &as[0], &textsize, &text, max::OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE);
		if (!err)
			str = text;
		else
			max::object_error(nullptr, "problem geting text from atoms");
		
		if (text)
			max::sysmem_freeptr(text);
		
		return str;
	}
	
	
	
	// TODO: move the atoms on return instead of copy
	atoms atoms_from_acav(long ac, max::t_atom* av) {
		atoms as;
		
		for (int i=0; i<ac; ++i)
			as.push_back(av[i]);
		return as;
	}	
	
}} // namespace c74::min
