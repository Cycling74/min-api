/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class timer;
	extern "C" void timer_tick_callback(timer* an_owner);
	
	
	/// The %timer class allows you to schedule a function to be called in the future using Max's scheduler.
	/// Note: the name `timer` was chosen instead of `clock` because of the use of the type is `clock` is ambiguous on the Mac OS
	/// unless you explicitly specify the `c74::min` namespace.
	
	class timer {
	public:

		timer(object_base* an_owner, function a_function)
		: m_owner(an_owner)
		, m_function(a_function)
		{
			m_instance = max::clock_new(this, (max::method)timer_tick_callback);
		}

		
		~timer() {
			object_free(m_instance);
		}
		
		
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
		
		
		/// post information about the timer to the console
		// also serves the purpose of eliminating warnings about m_owner being unused
		void post() {
			std::cout << m_instance << &m_function << m_owner << std::endl;
		}
		
		
	private:
		object_base*	m_owner;
		function		m_function;
		max::t_clock*	m_instance = nullptr;
		
	};
	
	
	void timer_tick_callback(timer* a_timer) {
		a_timer->tick();
	}

	
}} // namespace c74::min
