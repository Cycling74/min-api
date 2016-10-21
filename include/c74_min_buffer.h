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
		template<bool>
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
	

	/// A lock guard and accessor for buffer~ access from the audio thread.
	template<bool audio_thread_access = true>
	class buffer_lock {
	public:
		
		buffer_lock(buffer_reference& a_buffer_ref);
		~buffer_lock();


		bool valid() {
			if (!m_tab)
				return false;
			else
				return true;
		}
		
		
		size_t framecount() {
			return max::buffer_getframecount(m_buffer_obj);
		}
		
		
		int channelcount() {
			return (int)max::buffer_getchannelcount(m_buffer_obj);
		}
		
		
		float& operator[](long index) {
			return m_tab[index];
		}
		
		
		float& lookup(size_t frame, int channel = 0) {
			if (frame >= framecount())
				frame = framecount() - 1;
			
			auto index = frame;
			
			if (channelcount() > 1)
				index = index * channelcount() + channel;

			return m_tab[index];
		}


		double samplerate() {
			max::t_buffer_info info;

			max::buffer_getinfo(m_buffer_obj, &info);
			return info.b_sr;
		}


		double length_in_seconds() {
			return framecount() / samplerate();
		}

		void dirty() {
			max::buffer_setdirty(m_buffer_obj);
		}


		/// resize a buffer.
		/// only available for non-audio thread access.
		template<bool U=audio_thread_access, typename enable_if< U == false, int>::type = 0>
		void resize(double length_in_ms) {
			max::object_attr_setfloat(m_buffer_obj, k_sym_size, length_in_ms);
		}


	private:
		buffer_reference&	m_buffer_ref;
		max::t_buffer_obj*	m_buffer_obj	{ nullptr };
		float*				m_tab			{ nullptr };
	};


}} // namespace c74::min
