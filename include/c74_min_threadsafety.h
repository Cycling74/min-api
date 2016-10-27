/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	enum class thread_check {
		main,
		scheduler,
		any,
		none
	};

	enum class thread_action {
		assert,
		fifo,
		first,
		last
	};


	template<class T, thread_check>
	class thread_trigger;

	template<class T, thread_check check>
	void thread_trigger_callback(thread_trigger<T,check>* self);


	/// A base class that triggerd an action to be performed in the subclass
	/// in the specified thread.
	/// The subclass handles all details of the data storage / queueing / etc.
	/// as well as interfacing with the code that needs the delivery of the triggered data.

	template<class T, thread_check check>
	class thread_trigger {
	public:
		explicit thread_trigger(T a_baton)
		: m_baton { a_baton }
		{
			m_qelem = (max::t_qelem*)max::qelem_new(this, (max::method)thread_trigger_callback<T,check>);
		}

		virtual ~thread_trigger() {
			max::qelem_free(m_qelem);
		}

		void update_instance(T a_baton) {
			m_baton = a_baton;
		}

		void set() {
			max::qelem_set(m_qelem);
		}

		virtual void callback() = 0;

	protected:
		T				m_baton;
		max::t_qelem*	m_qelem;
	};


	template<class T, thread_check check>
	void thread_trigger_callback(thread_trigger<T,check>* self) {
		self->callback();
	}


	template<class T>
	class thread_trigger<T,thread_check::scheduler> {
	public:
		explicit thread_trigger(T a_baton)
		: m_baton { a_baton }
		{
			m_clock = max::clock_new(this, (max::method)thread_trigger_callback<T,thread_check::scheduler>);
		}

		virtual ~thread_trigger() {
			object_free(m_clock);
		}

		void update_instance(T a_baton) {
			m_baton = a_baton;
		}

		void set() {
			max::clock_fdelay(m_clock, 0);
		}

		virtual void callback() = 0;

		virtual void push(message_type a_type, const atoms& values) = 0;

	protected:
		T				m_baton;
		max::t_clock*	m_clock;
	};

}} // namespace c74::min
