/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	/// A reference to a buffer~ object.
	/// The buffer_reference automatically adds the management hooks required for your object to work with a buffer~.
	/// This includes adding a 'set' message and a 'dblclick' message as well as dealing with notifications and binding.

	class buffer_reference {
	public:
		friend class buffer_lock;

		// takes a single arg, but cannot be marked explicit unless we are willing to decorate all using code with a cast to this type
		// thus we ignore the advice of C.46 @ https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md

		buffer_reference(object_base* an_owner, const function& a_function = nullptr)
		: owner { *an_owner }
		, m_notification_callback { a_function }
		{}
		
		
		~buffer_reference() {
			object_free(instance);
		}
		
		
		void set(symbol name) {
			if (!instance)
				instance = max::buffer_ref_new(owner, name);
			else
				buffer_ref_set(instance, name);
		}
		
		
	private:
		max::t_buffer_ref*	instance { nullptr };
		object_base&		owner;
		function			m_notification_callback;
		
		message<> set_meth = { &owner, "set", "Choose a named buffer~ from which to read.",
			MIN_FUNCTION {
				set(args[0]);
				return {};
			}
		};
		
		message<> dblclick_meth = { &owner, "dblclick",
			MIN_FUNCTION {
				max::buffer_view(max::buffer_ref_getobject(instance));
				return {};
			}
		};
		
		message<> notify_meth = { &owner, "notify",
			MIN_FUNCTION {
				symbol	s = args[1];
				symbol	msg = args[2];
				void*	sender = args[3];
				void*	data = args[4];

				if (m_notification_callback) {
					if (msg == k_sym_globalsymbol_binding)
						m_notification_callback( { k_sym_binding } );
					else if (msg == k_sym_globalsymbol_unbinding)
						m_notification_callback( { k_sym_unbinding } );
					else if (msg == k_sym_buffer_modified)
						m_notification_callback( { k_sym_modified } );
				}
			
				return { (long)max::buffer_ref_notify(instance, s, msg, sender, data) };
			}
		};

	};
	
	
	class buffer_lock {
	public:
		
		buffer_lock(buffer_reference& a_buffer_ref)
		: buffer_ref { a_buffer_ref }
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
			return (int)max::buffer_getchannelcount(buffer_obj);
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


		double samplerate() {
			max::t_buffer_info info;

			max::buffer_getinfo(buffer_obj, &info);
			return info.b_sr;
		}


		double length_in_seconds() {
			return framecount() / samplerate();
		}


	private:
		buffer_reference&	buffer_ref;
		max::t_buffer_obj*	buffer_obj	{ nullptr };
		float*				tab			{ nullptr };
	};
	
}} // namespace c74::min
