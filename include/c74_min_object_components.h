/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {

	class inlet_base;
	class outlet_base;
	class argument_base;
	class message_base;
	class attribute_base;
	
	template<typename T, threadsafe threadsafety = threadsafe::no>
	class attribute;

	class maxobject_base {
	public:
		
		operator max::t_object*() {
			return &m_objstorage.maxobj;
		}

		operator max::t_pxobject*() {
			return &m_objstorage.mspobj;
		}

		operator void*() {
			return &m_objstorage.maxobj;
		}

		
	private:
		union storage {
			max::t_object	maxobj;
			max::t_pxobject	mspobj;
		};
		
		storage m_objstorage;
	};
	
	
	/// An object_base is a generic way to pass around a min::object as a min::object, while sharing common code,
	/// is actually sepcific to the the user's defined class due to template specialization.
	class object_base {
		static const constexpr long k_magic = 19740011004791;

	public:
		object_base()
		: m_state { (max::t_dictionary*)k_sym__pound_d, false }
		{
			create_inlets();
			create_outlets();
		}
		
		
		virtual ~object_base() {
			// TODO: free proxy inlets!
		}


		virtual bool is_jitter_class() = 0;
		

		int current_inlet() {
			return proxy_getinlet(static_cast<max::t_object*>(m_maxobj));
		}
		
		operator max::t_object* () const {
			return maxobj();
		}

		max::t_object* maxobj() const {
			if (m_min_magic == k_magic)
				return m_maxobj;
			else
				return nullptr;
		}
		
		void assign_instance(max::t_object* instance) {
			m_maxobj = instance;

			// this very much not ideal...
			// we have two ways objects are instantiated:
			//
			// 1. instantiated by Max (using placement new)
			// 2. instantiated some other way (not using placement now)
			//
			// When created by Max we need to have the member set for the 
			// max object prior to the call to the constructor.
			// But, if we are instatiated directly then that memory is uninitialized.
			//
			// One option would be to use a global and access that during the construction.
			// That solution is wrought with many obvious problems, so this solution was chosen
			// despite some different problems 
			// (e.g. the rare case where the magic number would be 
			//  randomly initialized to the correct value.)

			m_min_magic = k_magic;
		}
		
		
		auto inlets() -> std::vector<inlet_base*>& {
			return m_inlets;
		}
		
		auto outlets() -> std::vector<outlet_base*>& {
			return m_outlets;
		}
		
		void create_inlets();
		void create_outlets();


		void register_argument(argument_base* arg) {
			m_arguments.push_back(arg);
		}

		// defined in c74_min_argument.h
		void process_arguments(const atoms& args);

		auto arguments() const -> const std::vector<argument_base*>& {
			return m_arguments;
		}


		auto messages() -> std::unordered_map<std::string, message_base*>& {
			return m_messages;
		}
		auto messages() const -> const std::unordered_map<std::string, message_base*>& {
			return m_messages;
		}

		
		auto attributes() -> std::unordered_map<std::string, attribute_base*>& {
			return m_attributes;
		}
		auto attributes() const -> const std::unordered_map<std::string, attribute_base*>& {
			return m_attributes;
		}

		void postinitialize() {
			m_initialized = true;
			m_initializing = false;
		}
		
		bool initialized() {
			return m_initialized;
		}
		
		dict state() {
			return m_state;
		}
		
        void set_classname(symbol s) {
            m_classname = s;
        }
        
        symbol classname() {
            return m_classname;
        }
        
		/// Try to call a named message.
		/// @param	name	The name of the message to attempt to call.
		/// @param	args	Any args you wish to pass to the message call.
		/// @return			If the message doesn't exist an empty set of atoms.
		///					Otherwise the results of the message.
		atoms try_call(const std::string& name, const atoms& args = {});

		/// Try to call a named message.
		/// @param	name	The name of the message to attempt to call.
		/// @param	arg		A single atom arg you wish to pass to the message call.
		/// @return			If the message doesn't exist an empty set of atoms.
		///					Otherwise the results of the message.
		atoms try_call(const std::string& name, const atom& arg) {
			atoms as = {arg};
			return try_call(name, as);
		}
        
        bool has_call(const std::string& name) {
            auto found_message = m_messages.find(name);
            return (found_message != m_messages.end());
        }

	protected:
		max::t_object*										m_maxobj;		// initialized prior to placement new
		long												m_min_magic;	// should be valid if m_maxobj has been assigned
		bool												m_initializing = true;
		bool												m_initialized = false;
		std::vector<inlet_base*>							m_inlets;
		std::vector<outlet_base*>							m_outlets;
		std::vector<argument_base*>							m_arguments;
		std::unordered_map<std::string, message_base*>		m_messages;
		std::unordered_map<std::string, attribute_base*>	m_attributes;
		dict												m_state;
        symbol                                              m_classname; // what's typed in the max box
	};
	
	
	

	

	class sample_operator_base;
	class vector_operator_base;
	class matrix_operator_base;
	class gl_operator_base;
	
	
	// Wrap the C++ class together with the appropriate Max object header
	// Max object header is selected automatically using the type of the base class.
	
	template<class min_class_type, class=void>
	struct minwrap;
	
	
	template<class min_class_type>
	struct minwrap <min_class_type, typename enable_if<
		   !is_base_of< vector_operator_base, min_class_type>::value
		&& !is_base_of< sample_operator_base, min_class_type>::value
		&& !is_base_of< gl_operator_base, min_class_type>::value
	>::type > {
		maxobject_base	max_base;
		min_class_type	min_object;
		
		void setup() {
			min_object.create_inlets();
			min_object.create_outlets();
		}
		
		void cleanup() {}

		max::t_object* maxobj() {
			return max_base;
		}
	};
	
	
	// maxname may come in as an entire path because of use of the __FILE__ macro
	inline std::string deduce_maxclassname(const char* maxname) {
		std::string smaxname;
		
		const char* start = strrchr(maxname, '/');
		if (start)
			start += 1;
		else {
			start = strrchr(maxname, '\\');
			if (start)
				start += 1;
			else
				start = maxname;
		}
		
		const char* end = strstr(start, "_tilde.cpp");
		if (end) {
			smaxname.assign(start, end-start);
			smaxname += '~';
		}
		else {
			const char* end = strrchr(start, '.');
			if (!end)
				end = start + strlen(start);
			if (!strcmp(end, ".cpp"))
				smaxname.assign(start, end-start);
			else
				smaxname = start;
		}
		return smaxname;
	}
	
}} // namespace c74::min
