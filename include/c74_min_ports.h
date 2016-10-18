/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	

	class port {
	public:
		port(object_base* an_owner, const std::string& a_description, const std::string& a_type)
		: m_owner(an_owner)
		, m_description(a_description)
		, m_type(a_type)
		{}
		
		bool has_signal_connection() {
			return m_signal_connection;
		}
		
		void update_signal_connection(bool new_signal_connection_status) {
			m_signal_connection = new_signal_connection_status;
		}
		
		const std::string& type() {
			return m_type;
		}
		
		const std::string& description() {
			return m_description;
		}
		
	protected:
		object_base*	m_owner;
		std::string		m_description;
		std::string		m_type;
		bool			m_signal_connection { false };
	};


	class inlet_base : public port {
		friend void object_base::create_inlets();
	public:
		inlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: port { an_owner, a_description, a_type}
		{}
	private:
		void* m_instance { nullptr };
	};


	enum class inlet_trigger {
		automatic,
		hot,
		cold
	};

	template<inlet_trigger inlet_trigger_type = inlet_trigger::automatic>
	class inlet : public inlet_base {
	public:
		inlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: inlet_base { an_owner, a_description, a_type}
		{
			m_owner->inlets().push_back(this);
		}
	};



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


	using t_max_outlet = void*;


	template<typename outlet_type>
	void outlet_do_send(t_max_outlet maxoutlet, const outlet_type& value) {
		if (value[0].a_type == max::A_LONG || value[0].a_type == max::A_FLOAT)
			max::outlet_list(maxoutlet, nullptr, value.size(), (max::t_atom*)&value[0]);
		else
			max::outlet_anything(maxoutlet, value[0], value.size()-1, (max::t_atom*)&value[1]);
	}

	//	template<thread_check check_type, thread_action action_type>
	template<>
	void outlet_do_send<max::t_atom_long>(t_max_outlet maxoutlet, const max::t_atom_long& value) {
		max::outlet_int(maxoutlet, value);
	}

	//	template<thread_check check_type, thread_action action_type>
	template<>
	void outlet_do_send<double>(t_max_outlet maxoutlet, const double& value) {
		max::outlet_float(maxoutlet, value);
	}


// TODO: could use different defaults for DEBUG vs RELEASE
	template<thread_check check = thread_check::any, thread_action action = thread_action::assert>
	class outlet;

	template<thread_check check, thread_action action_type, typename outlet_type>
	void handle_unsafe_outlet_send(outlet<check,action_type>* outlet, outlet_type arg) {
		assert(false);
	}


	class outlet_base : public port {
		friend void object_base::create_outlets();

	public:
		outlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type)
		: port { an_owner, a_description, a_type}
		{}

	protected:
		t_max_outlet m_instance { nullptr };
	};


	template<thread_check check, thread_action action>
	class outlet : public outlet_base {

		/// utility: queue an argument of any type for output
		template<typename argument_type>
		void queue_argument(const argument_type& arg) noexcept {
			m_queued_output.push_back(arg);
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
		: outlet_base { an_owner, a_description, a_type}
		{
			m_owner->outlets().push_back(this);
		}
		
		void send(bool value) {
			if (safe()) outlet_do_send(m_instance, (max::t_atom_long)value);
			else		handle_unsafe_outlet_send(this, value);
		}

		void send(int value) {
			if (safe())	outlet_do_send(m_instance, (max::t_atom_long)value);
			else		handle_unsafe_outlet_send(this, value);
		}

		void send(long value) {
			if (safe())	outlet_do_send(m_instance, (max::t_atom_long)value);
			else		handle_unsafe_outlet_send(this, value);
		}

		void send(size_t value) {
			if (safe())	outlet_do_send(m_instance, (max::t_atom_long)value);
			else		handle_unsafe_outlet_send(this, value);
		}

		void send(float value) {
			if (safe())	outlet_do_send(m_instance, (double)value);
			else		handle_unsafe_outlet_send(this, value);
		}

		void send(double value) {
			if (safe())	outlet_do_send(m_instance, (double)value);
			else		handle_unsafe_outlet_send(this, value);
		}
		
		void send(const atoms& value) {
			if (value.empty())
				return;
			if (!safe()) {
				handle_unsafe_outlet_send(this, value);
				return;
			}
			outlet_do_send(m_instance, value);
		}


		/// Send values out through the outlet
		template<typename ...ARGS>
		void send(ARGS... args) {
			handle_arguments(args...);
			send(m_queued_output);
			m_queued_output.clear();
		}
		
	private:
		atoms m_queued_output;

		bool safe();

	};


}} // namespace c74::min
