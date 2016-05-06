/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class clock;
	extern "C" void clock_tick_callback(clock* an_owner);
	
	
	
	class clock {
	public:

		clock(object_base* an_owner, function a_function)
		: owner(an_owner)
		, function(a_function)
		{
			instance = max::clock_new(this, (max::method)clock_tick_callback);
		}

		
		~clock() {
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
	
	
	void clock_tick_callback(clock* an_owner) {
		an_owner->tick();
	}

	
}} // namespace c74::min
