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
			max::dictobj_dictionaryfromatoms(&instance, content.size(), &content[0]);
		}
		
		/// Create an unregistered dictionary
		/// optionally getting a handle to an old-school t_dictionary
		dict(max::t_dictionary* d = nullptr) {
			if (d == nullptr)
				instance = max::dictionary_new();
			else {
				max::object_retain(d);
				instance = d;
			}
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
		
/*
		atoms operator[](symbol key){
			long			argc = 0;
			max::t_atom*	argv = nullptr;
			auto			err = max::dictionary_getatoms(instance, key, &argc, &argv);
			auto			as = atoms_from_acav(argc, argv);
			
			if (err)
				; // TODO: handle the error somehow?  throw an exception?
			
			return as;
		};
*/	
		
		// bounds check: if key doesn't exist, throw
		atom_reference at(symbol key){
			long			argc = 0;
			max::t_atom*	argv = nullptr;
			auto			err = max::dictionary_getatoms(instance, key, &argc, &argv);
			auto			as = atom_reference(argc, argv);
			
			if (err)
				; // TODO: handle the error somehow?  throw an exception?
			
			return as;
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
		max::t_dictionary*	instance = nullptr;
	};
	
}} // namespace c74::min
