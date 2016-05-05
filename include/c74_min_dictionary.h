#pragma once

namespace c74 {
namespace min {
	
	
	class dict {
	public:
		
		/// Create (or reference an existing) dictionary by name
		dict(symbol name) {
			auto d = max::dictobj_findregistered_retain(name);
			
			if (!d) {	// didn't find a dictionary with that name, so create it
				d = max::dictionary_new();
				max::t_symbol* s = name;
				instance = max::dictobj_register(d, &s);
			}
		}

		
		/// Create an unregistered dictionary
		dict() {
			instance = max::dictionary_new();
		}
		
		
		/// Create an unregistered dictionary from dict-syntax
		dict(atoms content) {
			max::dictobj_dictionaryfromatoms(&instance, content.size(), &content[0]);
		}
		
		
		dict(atom an_atom_containing_a_dict) {
			auto a = (max::t_atom*)&an_atom_containing_a_dict;
			instance = (max::t_dictionary*)max::atom_getobj(a);
			max::object_retain(instance);
		}
		
	
		~dict() {
			object_free(instance);
		}
		
		
		
		dict& operator = (const dict& value) {
			max::dictionary_clone_to_existing(value.instance, instance);
			return *this;
		}
		

		dict& operator = (const atom& value) {
			auto a = (max::t_atom*)&value;
			if (max::atomisdictionary(a))
				instance = (max::t_dictionary*)max::atom_getobj(a);
			return *this;
		}
	
		
		symbol name() {
			return dictobj_namefromptr(instance);
		}
		
		
		bool valid() {
			return instance != nullptr;
		}
		
		
		void copyunique(const dict& source) {
			dictionary_copyunique(instance, source.instance);
		}
		
		
		void touch() {
			object_notify(instance, max::gensym("modified"), nullptr);
		}
		
		
	private:
		max::t_dictionary*	instance = nullptr;
	};
	
}} // namespace c74::min
