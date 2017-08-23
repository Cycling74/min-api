/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_min_api.h"

namespace c74 {
namespace min {


	#ifdef __APPLE__
	#pragma mark object_base
	#endif


	// implemented out-of-line because of bi-directional dependency of min::message<> and min::object_base
	
	atoms object_base::try_call(const std::string& name, const atoms& args) {
		auto found_message = m_messages.find(name);
		if (found_message != m_messages.end())
			return (*found_message->second)(args);
		return {};
	}


	// implemented out-of-line because of bi-directional dependency of min::argument<> and min::object_base

	void object_base::process_arguments(const atoms& args) {
		auto argcount = std::min(args.size(), m_arguments.size());

		for (auto i=0; i<argcount; ++i)
			(*m_arguments[i])(args[i]);
	}


	// inlets have to be created as a separate step (by the wrapper) because
	// max creates them from right-to-left
	// note that some objects will not call this function... i.e. dsp objects or other strongly-typed objects.

	void object_base::create_inlets() {
		if (m_inlets.empty())
			return;
		for (auto i = m_inlets.size()-1; i>0; --i)
			m_inlets[i]->m_instance = max::proxy_new(m_maxobj, static_cast<long>(i), nullptr);
	}

	
	// outlets have to be created as a separate step (by the wrapper) because
	// max creates them from right-to-left

	void object_base::create_outlets() {
		for (auto outlet = m_outlets.rbegin(); outlet != m_outlets.rend(); ++outlet)
			(*outlet)->create();
	}


	#ifdef __APPLE__
	#pragma mark -
	#pragma mark c-style callbacks
	#endif


	// c-style callback from the max kernel (clock for the min::timer class)

	void timer_tick_callback(timer* a_timer) {
		if (a_timer->should_defer())
			a_timer->defer();
		else
			a_timer->tick();
	}


	// c-style callback from the max kernel (qelem for the min::timer class)

	void timer_qfn_callback(timer* a_timer) {
		a_timer->tick();
	}


	// c-style callback from the max kernel (qelem for the min::queue class)

	void queue_qfn_callback(queue* a_queue) {
		a_queue->qfn();
	}


	#ifdef __APPLE__
	#pragma mark -
	#pragma mark symbol
	#endif

	// parts of the symbol class but must be defined after atom is defined

	symbol::symbol(const atom& value) {
		s = value;
	}


	symbol& symbol::operator = (const atom& value) {
		s = value;
		return *this;
	}


	#ifdef __APPLE__
	#pragma mark -
	#pragma mark outlet_call_is_safe
	#endif


	// specialized implementations of outlet_call_is_safe() used by outlet<> implementation

	template<>
	bool outlet_call_is_safe<thread_check::main>() {
		if (max::systhread_ismainthread())
			return true;
		else
			return false;
	};


	template<>
	bool outlet_call_is_safe<thread_check::scheduler>() {
		if (max::systhread_istimerthread())
			return true;
		else
			return false;
	};


	template<>
	bool outlet_call_is_safe<thread_check::any>() {
		if (max::systhread_ismainthread() || max::systhread_istimerthread())
			return true;
		else
			return false;
	};


	template<>
	bool outlet_call_is_safe<thread_check::none>() {
		return true;
	};


	#ifdef __APPLE__
	#pragma mark -
	#pragma mark vector_operator
	#endif
	

	// implementation of sample_operator-style calls made to a vector_operator

	template<placeholder vector_operator_placeholder_type>
	sample vector_operator<vector_operator_placeholder_type>::operator()(sample x) {
		sample			input_storage[1] {x};
		sample			output_storage[1] {};
		sample*			input {input_storage};
		sample*			output { output_storage };
		audio_bundle	input_bundle { &input, 1, 1 };
		audio_bundle	output_bundle { &output, 1, 1 };

		(*this)(input_bundle, output_bundle);

		return output[0];
	}


}} // namespace c74::min
