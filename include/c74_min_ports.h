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
	
	
	class inlet : public port {
		friend void object_base::create_inlets();
	public:
		inlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: port { an_owner, a_description, a_type}
		{
			m_owner->inlets().push_back(this);
		}
		
	private:
		void* m_instance { nullptr };
	};
	
	
	// inlets have to be created as a separate step because
	// max creates them from right-to-left
	// note that some objects will not call this function... i.e. dsp objects or other strongly-typed objects.

	void object_base::create_inlets() {
		if (m_inlets.empty())
			return;
		for (auto i = m_inlets.size()-1; i>0; --i)
			m_inlets[i]->m_instance = max::proxy_new(m_maxobj, i, nullptr);
	}
	
	
	class outlet  : public port {
		friend void object_base::create_outlets();
	public:
		outlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
		: port { an_owner, a_description, a_type}
		{
			m_owner->outlets().push_back(this);
		}
		
		void send(double value) {
			max::outlet_float(m_instance, value);
		}

		void send(symbol s1) {
			max::outlet_anything(m_instance, s1, 0, nullptr);
		}

		void send(std::string s1) {
			max::outlet_anything(m_instance, max::gensym(s1.c_str()), 0, nullptr);
		}

		void send(const char* s1) {
			max::outlet_anything(m_instance, max::gensym(s1), 0, nullptr);
		}

		void send(symbol s1, symbol s2) {
			atom a(s2);
			max::outlet_anything(m_instance, s1, 1, &a);
		}

		void send(symbol s1, double f2) {
			atom a(f2);
			max::outlet_anything(m_instance, s1, 1, &a);
		}
		
		void send(const atoms& as) {
			if (as[0].a_type == max::A_LONG || as[0].a_type == max::A_FLOAT)
				max::outlet_anything(m_instance, k_sym_list, as.size(), (max::t_atom*)&as[0]);
			else
				max::outlet_anything(m_instance, as[0], as.size()-1, (max::t_atom*)&as[1]);
		}
		
	private:
		void* m_instance { nullptr };
	};
	
	
	// outlets have to be created as a separate step because
	// max creates them from right-to-left
	void object_base::create_outlets() {
		for (auto outlet = m_outlets.rbegin(); outlet != m_outlets.rend(); ++outlet) {
			if ((*outlet)->type() == "")
				(*outlet)->m_instance = max::outlet_new(m_maxobj, nullptr);
			else
				(*outlet)->m_instance = max::outlet_new(m_maxobj, (*outlet)->type().c_str());
		}
	}

	
}} // namespace c74::min
