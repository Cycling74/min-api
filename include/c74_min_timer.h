/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class timer;
	extern "C" void timer_tick_callback(timer* an_owner);
	extern "C" void timer_qfn_callback(timer* a_timer);

	
	/// The timer class allows you to schedule a function to be called in the future using Max's scheduler.
	/// Note: the name `timer` was chosen instead of `clock` because of the use of the type is `clock` is ambiguous on the Mac OS
	/// unless you explicitly specify the `c74::min` namespace.
	
	class timer {
	public:

		enum class options {
			deliver_on_scheduler,
			defer_delivery
		};

		timer(object_base* an_owner, function a_function, options options = options::deliver_on_scheduler)
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		{
			m_instance = max::clock_new(this, reinterpret_cast<max::method>(timer_tick_callback));
			if (options == options::defer_delivery)
				m_qelem = max::qelem_new(this, reinterpret_cast<max::method>(timer_qfn_callback));
		}

		timer(object_base* an_owner, options options, function a_function)
		: timer(an_owner, a_function, options)
		{}

		
		~timer() {
			object_free(m_instance);
			if (m_qelem)
				max::qelem_free(m_qelem);
		}


		timer(const timer&) = delete;


		void delay(double duration_in_ms) {
			clock_fdelay(m_instance, duration_in_ms);
		}
		
		
		void stop() {
			if (m_instance)
				clock_unset(m_instance);
		}
		
		
		void tick() {
			atoms a;
			m_function(a);
		}


		bool should_defer() {
			return m_qelem;
		}

		
		void defer() {
			max::qelem_set(m_qelem);
		}
		
		
		/// post information about the timer to the console
		// also serves the purpose of eliminating warnings about m_owner being unused
		void post() {
			std::cout << m_instance << &m_function << m_owner << std::endl;
		}
		
		
	private:
		object_base*	m_owner;
		function		m_function;
		max::t_clock*	m_instance { nullptr };
		max::t_qelem*	m_qelem { nullptr };
	};
	
	

	
}} // namespace c74::min
