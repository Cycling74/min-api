/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	
	/// A standard callback function used throughout Min for various purposes.
	/// Typically this is provided to argument as a lamba function using the #MIN_FUNCTION macro.
	/// @param	as		A vector of atoms which may contain any arguments passed to your function.
	/// @param	inlet	The number (zero-based index) of the inlet at which the message was received, if relevant. Otherwise -1.
	/// @see		MIN_FUNCTION

	using function = std::function<atoms(const atoms& as, int inlet)>;


	/// Provide the correct lamba function prototype for the min::argument constructor.
	/// @see argument
	/// @see argument_function

	#define MIN_FUNCTION [this](const c74::min::atoms& args, int inlet) -> c74::min::atoms


	// Represents any type of message.
	// Used internally to allow heterogenous containers of messages for the Min class.

	class message_base {
	protected:

		// Constructor. See the constructor documention for min::message<> to get more details on the arguments.

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
					 || a_name == "mouseenter"
					 || a_name == "mouseleave"
					 || a_name == "mousedown"
					 || a_name == "mouseup"
					 || a_name == "mousedragdelta"
					 || a_name == "mousedoubleclick"
			) {
				m_type = message_type::cant;
			}

			m_name = name;
			m_owner->messages()[name] = this;	// add the message to the owning object's pile
		}

	public:

		// All messages must define what happens when you call them.

		virtual atoms operator ()(atoms args = {}, int inlet = -1) = 0;
		virtual atoms operator ()(atom arg, int inlet = -1) = 0;


		/// Return the Max C API message type constant for this message.
		/// @return The type of the message as a numeric constant.

		long type() const {
			return static_cast<long>(m_type);
		}


		/// Return the provided description for use in documentation generation, auto-complete, etc.
		/// @return	The description string supplied when the message was created.

		std::string description_string() const {
			return m_description;
		}


		/// Return the name of the message.
		/// @return	The symbolic name of the message.

		symbol name() const {
			return m_name;
		}

	protected:
		object_base*	m_owner;
		function		m_function;
		message_type	m_type { message_type::gimme };
		symbol			m_name;
		description		m_description;

		friend class object_base;

		void update_inlet_number(int& inlet) {
			if (inlet == -1) {
				if (m_owner->inlets().size() > 1) // avoid this potentially expensive call if there is only one inlet
					inlet = proxy_getinlet(static_cast<max::t_object*>(*m_owner));
				else
					inlet = 0;
			}
		}
	};
	

	class deferred_message;		// forward declaration (see below)


	/// A message.
	/// Messages (sometimes called Methods) in Max are how actions are triggered in objects.
	/// When you create a message in your Min class you provide the action that it should trigger as an argument,
	/// usually using a lambda function and the #MIN_FUNCTION macro.
	///
	/// By default, all messages are assumed to not be threadsafe and thus will defer themselves to the main thread if input has come from another thread.
	/// This behavior can be modified using the optional template parameter.
	/// DO NOT pass threadsafe::yes as the template parameter unless you are truly certain that you have made your code threadsafe.
	///
	/// @tparam		threadsafety	If your object has been written specifically and carefully to be threadsafe then you may pass the option parameter
	///								threadsafe::yes.
	///								Otherwise you should just accept the default and let Min handle the threadsafety for you.

	template<threadsafe threadsafety = threadsafe::no>
	class message : public message_base {
	public:

		/// Create a new message for a Min class.
		///
		/// @param	an_owner		The Min object instance that owns this outlet. Typically you should pass 'this'.
		/// @param	a_name			The name of the message. This is how users in Max will trigger the message action.
		/// @param	a_function		The function to be called when the message is received by your object.
		///							This is typically provided as a lamba function using the #MIN_FUNCTION definition.
		/// @param	a_description	Optional, but highly encouraged, description string to document the message.
		/// @param	a_type			Optional message type determines what kind of messages Max can send.
		///							In most cases you should _not_ pass anything here and accept the default.

		message(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {}, message_type type = message_type::gimme)
		: message_base(an_owner, a_name, a_function, a_description)
		{}


		/// Create a new message for a Min class.
		///
		/// @param	an_owner		The Min object instance that owns this outlet. Typically you should pass 'this'.
		/// @param	a_name			The name of the message. This is how users in Max will trigger the message action.
		/// @param	a_description	Optional, but highly encouraged, description string to document the message.
		/// @param	a_function		The function to be called when the message is received by your object.
		///							This is typically provided as a lamba function using the #MIN_FUNCTION definition.

		message(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message_base(an_owner, a_name, a_function, a_description)
		{}


		/// Call the message's action.
		/// @param	args	Optional arguments to send to the message's action.
		/// @return			Any return values will be returned as atoms.

		atoms operator ()(atoms args = {}, int inlet = -1) override {
			update_inlet_number(inlet);

			// this is the same as what happens in a defer() call
			if (max::systhread_ismainthread())
				return m_function(args, inlet);
			else {
				auto m = std::make_unique<deferred_message>(this, args, inlet);
				m->push(m);
			}
			return {};
		}


		/// Call the message's action.
		/// @param	arg		A single argument to send to the message's action.
		/// @return			Any return values will be returned as atoms.

		atoms operator ()(atom arg, int inlet = -1) override {
			atoms as { arg };
			return (*this)(as, inlet);
		}

	private:

		// Any messages received from outside the main thread will be deferred using the queue below.

		friend class deferred_message;
		std::queue< std::unique_ptr<deferred_message> >		m_deferred_messages;
	};


	// A wrapper for the deferred_message class that can be properly passed using the Max C API.

	struct t_deferred_message {
		max::t_object		obj;
		deferred_message*	mess;
	};

	static max::t_class* s_deferred_message_class = nullptr;

	inline void deferred_message_class_setup() {
		if (!s_deferred_message_class) {

			// this class has no 'new' or 'free' method.
			// the memory allocation is instead managed in the deferred_message class rather than in this wrapper

			s_deferred_message_class = max::class_new("min_deferred_message", nullptr, nullptr, sizeof(t_deferred_message), nullptr, 0, 0);
		}
	}


	// The actual deferred_message implementation in C++

	class deferred_message {
	public:

		// The constructor creates not only an instance of this deferred_message
		// but also a wrapper that inherits from max::t_object so that we can
		// put this object into a max::qelem.
		//
		// Each deferred message creates it's own qelem.
		// This makes ownership unambiguous but if an object is expecting to have wild amounts of deferred messages
		// then it would best off (from a computational efficiency standpoint) to implement a min::queue instead of using
		// this automatic safety feature.

		deferred_message(message<threadsafe::no>* an_owning_message, const atoms& args, int inlet)
		: m_owning_message { an_owning_message }
		, m_args { args }
		, m_inlet { inlet }
		{
			deferred_message_class_setup(); // TODO: would be nice to avoid this lazy initialization?
			m_maxwrapper = static_cast<t_deferred_message*>(max::object_alloc(s_deferred_message_class));
			m_maxwrapper->mess = this;
			m_qelem = max::qelem_new(reinterpret_cast<max::t_object*>(m_maxwrapper), reinterpret_cast<max::method>(deferred_message::callback));
		}


		deferred_message(const deferred_message& other) = delete; // no copying allowed -- only moving!


		// destructor -- must also cleanup the Max wrapper around ourselves

		~deferred_message() {
			max::qelem_free(m_qelem);
			max::object_free(m_maxwrapper);
		}


		// move a message onto the queue of deferred messages

		void push(std::unique_ptr<deferred_message>& m) {
			m_owning_message->m_deferred_messages.push( std::move(m) );
			max::qelem_set(m_qelem);
		}


		// call a message's action and remove it from the queue
		// will free the deferred message in the process

		void pop() {
			m_owning_message->m_function(m_args, m_inlet);
			m_owning_message->m_deferred_messages.pop();
		}


	private:

		message<threadsafe::no>*	m_owning_message;
		atoms						m_args;
		t_deferred_message*			m_maxwrapper;
		max::t_qelem*				m_qelem;
		int							m_inlet;


		// C-api callback from the Max kernel when our qelem is serviced

		static void callback(t_deferred_message* self) {
			self->mess->pop();
		}
	};


	// specialization of message for messages which declare themselves to be threadsafe

	template<>
	class message<threadsafe::yes> : public message_base {
	public:

		/// Create a new message for a Min class.
		///
		/// @param	an_owner		The Min object instance that owns this outlet. Typically you should pass 'this'.
		/// @param	a_name			The name of the message. This is how users in Max will trigger the message action.
		/// @param	a_function		The function to be called when the message is received by your object.
		///							This is typically provided as a lamba function using the #MIN_FUNCTION definition.
		/// @param	a_description	Optional, but highly encouraged, description string to document the message.
		/// @param	a_type			Optional message type determines what kind of messages Max can send.
		///							In most cases you should _not_ pass anything here and accept the default.

		message(object_base* an_owner, const std::string& a_name, const function& a_function, const description& a_description = {}, message_type a_type = message_type::gimme)
		: message_base(an_owner, a_name, a_function, a_description, a_type)
		{}


		/// Create a new message for a Min class.
		///
		/// @param	an_owner		The Min object instance that owns this outlet. Typically you should pass 'this'.
		/// @param	a_name			The name of the message. This is how users in Max will trigger the message action.
		/// @param	a_description	Optional, but highly encouraged, description string to document the message.
		/// @param	a_function		The function to be called when the message is received by your object.
		///							This is typically provided as a lamba function using the #MIN_FUNCTION definition.

		message(object_base* an_owner, const std::string& a_name, const description& a_description, const function& a_function)
		: message_base(an_owner, a_name, a_function, a_description)
		{}


		/// Call the message's action.
		/// @param	args	Optional arguments to send to the message's action.
		/// @param	inlet	Optional inlet number associated with the incoming message.
		/// @return			Any return values will be returned as atoms.

		atoms operator ()(atoms args = {}, int inlet = -1) override {
			update_inlet_number(inlet);
			return m_function(args, inlet);
		}


		/// Call the message's action.
		/// @param	arg		A single argument to send to the message's action.
		/// @param	inlet	Optional inlet number associated with the incoming message.
		/// @return			Any return values will be returned as atoms.

		atoms operator ()(atom arg, int inlet = -1) override {
			return m_function( { arg }, inlet );
		}

	};

}} // namespace c74::min
