/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	// this is the peer object for the t_hashtab in the legacy max api
	template<class T>
	class map {
	public:
		
		map() {
			instance = c74::max::hashtab_new(0);
			c74::max::hashtab_flags(instance, c74::max::OBJ_FLAG_DATA);
		}
		
		
		~map() {
			// TODO: Iterate and delete? Provide callback for cleanup?
			object_free(instance);
		}
		
		
		T operator[](symbol key) {
			T value = nullptr;
			
			auto err = hashtab_lookup(instance, key, (max::t_object**)&value);
			if (err) {
				// TODO: ??? throw an exception ???
				return nullptr;
			}
			
			return value;
		};

		
		bool exists(symbol key) {
			T value = nullptr;
			
			auto err = hashtab_lookup(instance, key, (max::t_object**)&value);
			if (err)
				return false;
			else
				return true;
		}
		
		
		void store(symbol key, T value) {
			/*auto err = */max::hashtab_store(instance, key, (max::t_object*)value);
			// TODO: handle errors
		}
		
		
	private:
		max::t_hashtab*	instance { nullptr };
	};
	
}} // namespace c74::min
