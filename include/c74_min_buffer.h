/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class buffer_reference {
	public:
		friend class buffer_lock;
		
		buffer_reference(object_base* an_owner)
		: owner(an_owner)
		{}
		
		
		~buffer_reference() {
			object_free(instance);
		}
		
		
		void set(symbol name) {
			if (!instance)
				instance = buffer_ref_new(owner->maxobj, name);
			else
				buffer_ref_set(instance, name);
		}
		
		
	private:
		max::t_buffer_ref*	instance = nullptr;
		object_base*		owner;
		
		c74::min::method set_meth = { owner, "set", [this](c74::min::atoms& args) {
			this->set(args[0]);
		}};
		
		c74::min::method dblclick_meth = { owner, "dblclick", [this](c74::min::atoms& args) {
			max::buffer_view(max::buffer_ref_getobject(this->instance));
		}};
		
		c74::min::method notify_meth = { owner, "notify", [this](c74::min::atoms& args) {
			//max::t_object*	self = args[0];
			max::t_symbol*	s = args[1];
			max::t_symbol*	msg = args[2];
			void*			sender = args[3];
			void*			data = args[4];
			
			return buffer_ref_notify(instance, s, msg, sender, data);
		}};
		

	};
	
	
	class buffer_lock {
	public:
		
		buffer_lock(buffer_reference& a_buffer_ref)
		: buffer_ref(a_buffer_ref)
		{
			buffer_obj = buffer_ref_getobject(buffer_ref.instance);
			tab = buffer_locksamples(buffer_obj);
			// TODO: handle case where tab is null -- can't throw an exception in audio code...
		}
		
		
		~buffer_lock() {
			if (tab)
				buffer_unlocksamples(buffer_obj);
		}
		
		
		bool valid() {
			if (!tab)
				return false;
			else
				return true;
		}
		
		
		size_t framecount() {
			return max::buffer_getframecount(buffer_obj);
		}
		
		
		int channelcount() {
			return max::buffer_getchannelcount(buffer_obj);
		}
		
		
		float& operator[](long index) {
			return tab[index];
		}
		
		
		float& lookup(size_t frame, int channel = 0) {
			if (frame >= framecount())
				frame = framecount() - 1;
			
			auto index = frame;
			
			if (channelcount() > 1)
				index = index * channelcount() + channel;

			return tab[index];
		}
		
	private:
		buffer_reference&	buffer_ref;
		max::t_buffer_obj*	buffer_obj = nullptr;
		float*				tab = nullptr;
	};
	
}} // namespace c74::min
