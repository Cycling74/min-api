/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

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
		
		/// Create an unregistered dictionary from dict-syntax
		dict(atoms content) {
			max::dictobj_dictionaryfromatoms(&instance, (long)content.size(), &content[0]);
		}
		
		/// Create an unregistered dictionary
		/// @param d				optionally getting a handle to an old-school t_dictionary
		/// @param take_ownership	defaults to true, change to false only in exceptional cases
		dict(max::t_dictionary* d = nullptr, bool take_ownership = true) {
			if (d == nullptr)
				instance = max::dictionary_new();
			else {
				if (take_ownership)
					max::object_retain(d);
				else
					has_ownership = false;
				instance = d;
			}
		}
		
		
		dict(atom an_atom_containing_a_dict) {
			auto a = (max::t_atom*)&an_atom_containing_a_dict;
			instance = (max::t_dictionary*)max::atom_getobj(a);
			if (!instance)
				error("no dictionary in atom");
			auto err = max::object_retain(instance);
			if (err)
				error("failed to retain dictionary instance");
		}
		
	
		~dict() {
			if (has_ownership)
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
		
		
		// bounds check: if key doesn't exist, throw
		atom_reference at(symbol key){
			long			argc = 0;
			max::t_atom*	argv = nullptr;
			auto			err = max::dictionary_getatoms(instance, key, &argc, &argv);
			
			if (err)
				error("could not get key from dictionary");
			return atom_reference(argc, argv);
		}
		
		
		// bounds check: if key doesn't exist, create it
		atom_reference operator[](symbol key){
			if (!max::dictionary_hasentry(instance, key))
				max::dictionary_appendatom(instance, key, &atoms{0}[0]);
			return at(key);
		};

		
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
			object_notify(instance, k_sym_modified, nullptr);
		}
		
		
	private:
		max::t_dictionary*	instance		{ nullptr };
		bool				has_ownership	{ true };
	};
	
}} // namespace c74::min
