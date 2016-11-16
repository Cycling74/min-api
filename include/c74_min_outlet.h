/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	using t_max_outlet = void*;


	template<typename outlet_type>
	inline void outlet_do_send(t_max_outlet maxoutlet, const outlet_type& value) {
		if (value[0].a_type == max::A_LONG || value[0].a_type == max::A_FLOAT)
			max::outlet_list(maxoutlet, nullptr, value.size(), static_cast<const max::t_atom*>(&value[0]));
		else {
			if (value.size() > 1)
				max::outlet_anything(maxoutlet, value[0], value.size()-1, static_cast<const max::t_atom*>(&value[1]));
			else
				max::outlet_anything(maxoutlet, value[0], 0, nullptr);
		}
	}

	//	template<thread_check check_type, thread_action action_type>
	template<>
	inline void outlet_do_send<max::t_atom_long>(t_max_outlet maxoutlet, const max::t_atom_long& value) {
		max::outlet_int(maxoutlet, value);
	}

	//	template<thread_check check_type, thread_action action_type>
	template<>
	inline void outlet_do_send<double>(t_max_outlet maxoutlet, const double& value) {
		max::outlet_float(maxoutlet, value);
	}


	/// default thread_action is to assert, which means no queue at all...
	template<thread_check check, thread_action action>
	class outlet_queue : public thread_trigger<t_max_outlet,check> {
	public:
		explicit outlet_queue(t_max_outlet a_maxoutlet)
		: thread_trigger<t_max_outlet,check> ( a_maxoutlet )
		{}

		void callback() {}
	};


	/// store only the first value and discard additional values (opposite of usurp)
	template<thread_check check>
	class outlet_queue<check, thread_action::first> : public thread_trigger<t_max_outlet,check> {
	public:
		explicit outlet_queue(t_max_outlet a_maxoutlet)
		: thread_trigger<t_max_outlet,check> ( a_maxoutlet )
		{}

		void callback() {
			outlet_do_send(m_value);
			m_set = false;
		}

		void push(message_type a_type, const atoms& as) {
			if (!m_set) {
				m_value = as;
				m_set = true;
				thread_trigger<t_max_outlet,check>::set();
			}
		}

	private:
		atoms	m_value;
		bool	m_set { false };
	};


	/// store only the last value received (usurp)
	template<thread_check check>
	class outlet_queue<check, thread_action::last> : public thread_trigger<t_max_outlet,check> {
	public:
		explicit outlet_queue(t_max_outlet a_maxoutlet)
		: thread_trigger<t_max_outlet,check> ( a_maxoutlet )
		{}

		void callback() {
			outlet_do_send(this->m_maxoutlet, m_value);
		}

		void push(message_type a_type, const atoms& as) {
			m_value = as;
			thread_trigger<t_max_outlet,check>::set();
		}

	private:
		atoms	m_value;
	};


	/// defer all values
	template<thread_check check>
	class outlet_queue<check, thread_action::fifo> : public thread_trigger<t_max_outlet,check> {

		struct tagged_atoms {
			message_type	m_type;
			atoms			m_as;
		};

	public:
		explicit outlet_queue(t_max_outlet a_maxoutlet)
		: thread_trigger<t_max_outlet,check> ( a_maxoutlet )
		{}

		void callback() {
			tagged_atoms tas;
			while (m_values.try_dequeue(tas)) {
				if (tas.m_type == message_type::long_arg)
					outlet_do_send<max::t_atom_long>(this->m_baton, tas.m_as[0]);
				else if (tas.m_type == message_type::float_arg)
					outlet_do_send<double>(this->m_baton, tas.m_as[0]);
				else
					outlet_do_send(this->m_baton, tas.m_as);
			}
		}

		void push(message_type a_type, const atoms& as) {
			tagged_atoms tas { a_type, as };
			m_values.enqueue(tas);
			thread_trigger<t_max_outlet,check>::set();
		}


	private:
		fifo<tagged_atoms>	m_values;
	};


	template<thread_check>
	bool outlet_call_is_safe();


	// for performance reasons we do not check threads in release builds unless specifically requested.
	// we do check it in debug builds, however (unless specifically not requested).

#ifdef NDEBUG
	template<thread_check check = thread_check::none, thread_action action = thread_action::assert>
	class outlet;
#else // DEBUG
	template<thread_check check = thread_check::any, thread_action action = thread_action::assert>
	class outlet;
#endif



	template<thread_check check, thread_action action_type, typename outlet_type>
	class handle_unsafe_outlet_send;


	template<thread_check check, typename outlet_type>
	class handle_unsafe_outlet_send<check, thread_action::assert, outlet_type> {
	public:
		handle_unsafe_outlet_send(outlet<check,thread_action::assert>* an_outlet, const outlet_type& a_value) {
			assert(false);
		}
	};


	template<thread_check check, typename outlet_type>
	class handle_unsafe_outlet_send<check, thread_action::fifo, outlet_type> {
	public:
		handle_unsafe_outlet_send(outlet<check,thread_action::fifo>* an_outlet, const outlet_type& a_value) {
			if (typeid(outlet_type) == typeid(max::t_atom_long))
				an_outlet->queue_storage().push(message_type::long_arg, a_value);
			else if (typeid(outlet_type) == typeid(double))
				an_outlet->queue_storage().push(message_type::float_arg, a_value);
			else // atoms
				an_outlet->queue_storage().push(message_type::gimme, a_value);
		}
	};



	class outlet_base : public port {
		friend void object_base::create_outlets();

	public:
		outlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type)
		: port ( an_owner, a_description, a_type )
		{}

		virtual void create() = 0;

	protected:
		t_max_outlet m_instance { nullptr };
	};


	template<thread_check check, thread_action action>
	class outlet : public outlet_base {
		/// utility: queue an argument of any type for output
		template<typename argument_type>
		void queue_argument(const argument_type& arg) noexcept {
			m_accumulated_output.push_back(arg);
		}
		
		/// utility: empty argument handling (required for all recursive variadic templates)
		void handle_arguments() noexcept {
			;
		}
		
		/// utility: handle N arguments of any type by recursively working through them
		///	and matching them to the type-matched routine above.
		template <typename FIRST_ARG, typename ...REMAINING_ARGS>
		void handle_arguments(FIRST_ARG const& first, REMAINING_ARGS const& ...args) noexcept {
			queue_argument(first);
			if (sizeof...(args))
				handle_arguments(args...); // recurse
		}
		
	public:
		outlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: outlet_base ( an_owner, a_description, a_type )
		{
			m_owner->outlets().push_back(this);
		}

		void create() {
			if (type() == "")
				m_instance = max::outlet_new(m_owner->maxobj(), nullptr);
			else
				m_instance = max::outlet_new(m_owner->maxobj(), type().c_str());
			m_queue_storage.update_instance(m_instance);
		}

		void send(bool value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (max::t_atom_long)value);
			else
				handle_unsafe_outlet_send<check,action,max::t_atom_long>(this, value);
		}

		void send(int value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (max::t_atom_long)value);
			else
				handle_unsafe_outlet_send<check,action,max::t_atom_long>(this, value);
		}

		void send(long value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (max::t_atom_long)value);
			else
				handle_unsafe_outlet_send<check,action,max::t_atom_long>(this, value);
		}

		void send(size_t value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (max::t_atom_long)value);
			else
				handle_unsafe_outlet_send<check,action,max::t_atom_long>(this, value);
		}

		void send(float value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (double)value);
			else
				handle_unsafe_outlet_send<check,action,double>(this, value);
		}

		void send(double value) {
			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, (double)value);
			else
				handle_unsafe_outlet_send<check,action,double>(this, value);
		}
		
		void send(const atoms& value) {
			if (value.empty())
				return;

			if (outlet_call_is_safe<check>())
				outlet_do_send(m_instance, value);
			else
				handle_unsafe_outlet_send<check,action,atoms>(this, value);
		}


		/// Send values out through the outlet
		template<typename ...ARGS>
		void send(ARGS... args) {
			handle_arguments(args...);
			send(m_accumulated_output);
			m_accumulated_output.clear();
		}

		outlet_queue<check,action>& queue_storage() {
			return m_queue_storage;
		}

	private:
		atoms						m_accumulated_output;
		outlet_queue<check,action>	m_queue_storage { this->m_instance };


	};


}} // namespace c74::min
