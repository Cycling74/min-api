/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {
	





	class queue;
	extern "C" void queue_qfn_callback(queue* a_queue);


	/// The queue class allows you to defer the call of a function to the near future in
	/// Max's main (low-priority) thread.

	class queue {
	public:

		queue(object_base* an_owner, function a_function)
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		{
			m_instance = max::qelem_new(this, (max::method)queue_qfn_callback);
		}


		~queue() {
			max::qelem_free(m_instance);
		}


		queue(const timer&) = delete;


		void set() {
			max::qelem_set(m_instance);
		}


		void unset() {
			max::qelem_unset(m_instance);
		}


		void qfn() {
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
		void*			m_instance { nullptr };
	};
	
	




}} // namespace c74::min
