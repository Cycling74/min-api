/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class timer;
	extern "C" void timer_tick_callback(timer* an_owner);
	
	
	/// The #timer class allows you to schedule a function to be called in the future using Max's scheduler.
	/// Note: the name `timer` was chosen instead of `clock` because of the use of the type is `clock` is ambiguous on the Mac OS
	/// unless you explicitly specify the `c74::min` namespace.
	
	class timer {
	public:

		timer(object_base* an_owner, function a_function)
		: owner(an_owner)
		, function(a_function)
		{
			instance = max::clock_new(this, (max::method)timer_tick_callback);
		}

		
		~timer() {
			object_free(instance);
		}
		
		
		void delay(double duration_in_ms) {
			clock_fdelay(instance, duration_in_ms);
		}
		
		
		void stop() {
			if (instance)
				clock_unset(instance);
		}
		
		
		void tick() {
			atoms a;
			function(a);
		}
		
		
//		void operator ()(atoms args) {
//			function(args);
//		}
//
//		void operator ()() {
//			atoms a;
//			function(a);
//		}
		
		//private:
		object_base*	owner;
		function		function;
		max::t_clock*	instance = nullptr;
		
	};
	
	
	void timer_tick_callback(timer* an_owner) {
		an_owner->tick();
	}

	
}} // namespace c74::min
