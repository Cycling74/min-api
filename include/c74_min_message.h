/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	using function = std::function<atoms(const atoms&)>;
	#define MIN_FUNCTION [this](const c74::min::atoms& args) -> c74::min::atoms


	class message_base {
	public:

		message_base(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {}, message_type type = message_type::gimme)
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		, m_type		{ type }
		, m_description	{ a_description }
		{
			assert(m_function != nullptr); // could happen if a function is passed as the arg but that fn hasn't initialized yet

			std::string name = a_name;
			
			if (name == "integer") {
				name = "int";
				m_type = message_type::long_arg;
			}
			else if (name == "number") {
				name = "float";
				m_type = message_type::float_arg;
			}
			else if (   a_name == "dblclick"
					 || a_name == "dsp64"
					 || a_name == "dspsetup"
					 || a_name == "edclose"
                     || a_name == "fileusage"
					 || a_name == "jitclass_setup"
					 || a_name == "maxclass_setup"
					 || a_name == "maxob_setup"
					 || a_name == "mop_setup"
					 || a_name == "notify"
					 || a_name == "okclose"
					 || a_name == "patchlineupdate"
					 || a_name == "savestate"
					 || a_name == "setup"
			) {
				m_type = message_type::cant;
			}

			m_name = name;
			m_owner->messages()[name] = this;
		}

		message_base(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message_base(an_owner, a_name, a_function, a_description)
		{}

		virtual atoms operator ()(atoms args = {}) = 0;
		virtual atoms operator ()(atom arg) = 0;

		long type() const {
			return static_cast<long>(m_type);
		}

		std::string description_string() const {
			return m_description;
		}

		symbol name() const {
			return m_name;
		}

	protected:
		object_base*	m_owner;
		function		m_function;
		message_type	m_type { message_type::gimme };
		symbol			m_name;
		description		m_description;
	};
	


	class deferred_message;


	template<threadsafe threadsafety = threadsafe::no>
	class message : public message_base {
	public:
		friend class deferred_message;

		message(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {}, message_type type = message_type::gimme)
		: message_base(an_owner, a_name, a_function, a_description)
		{}

		message(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message_base(an_owner, a_name, a_function, a_description)
		{}

		atoms operator ()(atoms args = {}) override {
			// this is the same as what happens in a defer() call
			if (max::systhread_ismainthread())
				return m_function(args);
			else {
				auto m = std::make_unique<deferred_message>(this, args);
				m->push(m);
			}
			return {};
		}

		atoms operator ()(atom arg) override {
			atoms as { arg };
			return (*this)(as);
		}

	private:
		std::queue< std::unique_ptr<deferred_message> >		m_deferred_messages;
	};



	struct t_deferred_message {
		max::t_object		obj;
		deferred_message*	mess;
	};

	static max::t_class* s_deferred_message_class = nullptr;

	inline void deferred_message_class_setup() {
		if (!s_deferred_message_class)
			s_deferred_message_class = max::class_new("min_deferred_message", nullptr, nullptr, sizeof(t_deferred_message), nullptr, 0, 0);
	}

	template<threadsafe threadsafety>
	class message;

	class deferred_message {
	public:
		deferred_message(message<threadsafe::no>* owner, const atoms& args)
		: m_owner { owner }
		, m_args { args }
		{
			deferred_message_class_setup(); // TODO: would be nice to avoid this lazy initialization?
			m_maxwrapper = static_cast<t_deferred_message*>(max::object_alloc(s_deferred_message_class));
			m_maxwrapper->mess = this;
			m_qelem = max::qelem_new(reinterpret_cast<max::t_object*>(m_maxwrapper), reinterpret_cast<max::method>(deferred_message::callback));
		}

		deferred_message(const deferred_message& other) = delete; // no copying allowed!

		~deferred_message() {
			max::qelem_free(m_qelem);
		}

		void push(std::unique_ptr<deferred_message>& m) {
			m_owner->m_deferred_messages.push( std::move(m) );
			max::qelem_set(m_qelem);
		}

		void pop() {
			m_owner->m_function(m_args);
			m_owner->m_deferred_messages.pop();
		}

		static void callback(t_deferred_message* self) {
			self->mess->pop();
		}

	private:
		message<threadsafe::no>*	m_owner;
		atoms						m_args;
		t_deferred_message*			m_maxwrapper;
		max::t_qelem*				m_qelem;
	};



	template<>
	class message<threadsafe::yes> : public message_base {
	public:

		message(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {}, message_type type = message_type::gimme)
		: message_base(an_owner, a_name, a_function, a_description)
		{}

		message(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message_base(an_owner, a_name, a_function, a_description)
		{}

		atoms operator ()(atoms args = {}) override {
			return m_function(args);
		}

		atoms operator ()(atom arg) override {
			return m_function( { arg } );
		}

	};





}} // namespace c74::min
