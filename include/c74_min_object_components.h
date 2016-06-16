/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <sstream>

namespace c74 {
namespace min {
	
	
	class inlet;
	class outlet;
	class method;
	class attribute_base;
	
	template <typename T>
	class attribute;
	
	
	class maxobject_base {
	public:
		
		operator max::t_pxobject*() {
			return &objstorage.mspobj;
		}

		operator void*() {
			return &objstorage.maxobj;
		}

		
	private:
		union storage {
			max::t_object	maxobj;
			max::t_pxobject	mspobj;
		};
		
		//enum class type {
		//	max = 0,
		//	msp,
		//};
		//
		//type objtype = type::max;
		storage objstorage;
		
	};
	
	
	
	class object_base {
	public:
		object_base()
		: state((max::t_dictionary*)k_sym__pound_d) {
		}
		
		~object_base() {
		}

		int current_inlet() {
			return proxy_getinlet((max::t_object*)maxobj);
		}
        
		logger post(logger::type target = logger::type::message) {
            return logger(maxobj, target);
        }
		
		max::t_object*										maxobj;
		bool												initialized = false;
		std::vector<min::inlet*>							inlets;
		std::vector<min::outlet*>							outlets;
		std::unordered_map<std::string, method*>			methods;
		std::unordered_map<std::string, attribute_base*>	attributes;
		dict												state;
	};
	
	
	class object : public object_base {
		;
	};

	class sample_operator_base;
	class perform_operator_base;
	class matrix_operator_base;
	class gl_operator_base;
	
	
	// Wrap the C++ class together with the appropriate Max object header
	// Max object header is selected automatically using the type of the base class.
	
	template<class T, class=void>
	struct minwrap;
	
	
	template<class T>
	struct minwrap < T, typename std::enable_if<
		!std::is_base_of< min::perform_operator_base, T>::value
		&& !std::is_base_of< min::sample_operator_base, T>::value
		&& !std::is_base_of< min::gl_operator_base, T>::value
	>::type > {
		maxobject_base	base;
		T				obj;
		
		void setup() {
			auto self = &base;

			for (auto i=obj.inlets.size()-1; i>0; --i)
				obj.inlets[i]->instance = max::proxy_new(self, i, nullptr);
		}
		
		void cleanup() {}
	};
	
	
#pragma mark -
#pragma mark inlets / outlets

	class port {
	public:
		port(object_base* an_owner, std::string a_description, std::string a_type)
		: owner(an_owner)
		, description(a_description)
		, type(a_type)
		{}
		
		bool has_signal_connection() {
			return signal_connection;
		}
		
		object_base*	owner;
		std::string		description;
		std::string		type;
		bool			signal_connection = false;
	};
	
	
	class inlet : public port {
	public:
		//		inlet(maxobject* an_owner, std::string a_description)
		inlet(object_base* an_owner, std::string a_description, std::string a_type = "")
		: port(an_owner, a_description, a_type)
		{
			owner->inlets.push_back(this);
		}

		void* instance = nullptr;
	};
	
	
	class outlet  : public port {
	public:
		outlet(object_base* an_owner, std::string a_description, std::string a_type = "")
		: port(an_owner, a_description, a_type)
		{
			owner->outlets.push_back(this);
		}
		
		void send(double value) {
			c74::max::outlet_float(instance, value);
		}

		void send(symbol s1) {
			c74::max::outlet_anything(instance, s1, 0, nullptr);
		}

		void send(std::string s1) {
			c74::max::outlet_anything(instance, c74::max::gensym(s1.c_str()), 0, nullptr);
		}

		void send(const char* s1) {
			c74::max::outlet_anything(instance, c74::max::gensym(s1), 0, nullptr);
		}

		void send(symbol s1, symbol s2) {
			atom a(s2);
			c74::max::outlet_anything(instance, s1, 1, &a);
		}

		void send(symbol s1, double f2) {
			atom a(f2);
			c74::max::outlet_anything(instance, s1, 1, &a);
		}
		
		void* instance = nullptr;
	};
	
#ifdef __APPLE__
# pragma mark -
# pragma mark methods
#endif

	using function = std::function<void(atoms&)>;
	
	class method {
	public:
		method(object_base* an_owner, std::string a_name, function a_function)
		: owner(an_owner)
		, function(a_function)
		{
			if (a_name == "integer")
				a_name = "int";
			else if (a_name == "number")
				a_name = "float";
			else if (a_name == "dsp64" || a_name == "dblclick" || a_name == "edclose" || a_name == "okclose")
				type = max::A_CANT;
			owner->methods[a_name] = this;
		}
		
		void operator ()(atoms args) {
			function(args);
		}
		
		void operator ()(atom arg) {
			atoms as = { arg };
			function(as);
		}
		
		void operator ()() {
			atoms as;
			function(as);
		}
		
		//private:
		object_base*	owner;
		long			type = max::A_GIMME;
		function		function;
	};

	
	class attribute_base {
	public:
		attribute_base(object_base* an_owner, std::string a_name, function a_function)
		: owner(an_owner)
		, name(max::gensym(a_name.c_str()))
		, setter(a_function)
		{}
		
		virtual attribute_base& operator = (atoms args) = 0;
		virtual operator atoms() const = 0;
		
		object_base*	owner;
		max::t_symbol*	name;
		symbol			type;
		function		setter;
	};
	
	template <typename T>
	class attribute : public attribute_base {
	public:
		attribute(object_base* an_owner, std::string a_name, T a_default_value, function a_function)
		: attribute_base(an_owner, a_name, a_function)
		{
			owner->attributes[a_name] = this;
			*this = a_default_value;
			
			if (std::is_same<T, bool>::value)
				type = k_sym_long;
			else if (std::is_same<T, int>::value)
				type = k_sym_long;
			else if (std::is_same<T, symbol>::value)
				type = k_sym_symbol;
			else if (std::is_same<T, float>::value)
				type = k_sym_float32;
			else // (std::is_same<T, double>::value)
				type = k_sym_float64;

		}
		
		
		attribute& operator = (const T arg) {
			atoms as = { atom(arg) };
			*this = as;
			if (owner->initialized)
				object_attr_touch(owner->maxobj, name);
			return *this;
		}
		
		
		attribute& operator = (atoms args) {
			if (setter)
				setter(args);
			value = args[0];
			return *this;
		}
		
		
		operator atoms() const {
			atom a = value;
			atoms as = { a };
			return as;
		}
		
		
		operator T() const {
			return value;
		}
		
		
	private:
		T				value;
	};
	
	
	
	
	
	static max::t_symbol* ps_getname = max::gensym("getname");
	

	
	template<class T>
	max::t_max_err min_attr_getter(minwrap<T>* self, max::t_object* maxattr, long* ac, max::t_atom** av) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, ps_getname);
		auto& attr = self->obj.attributes[attr_name->s_name];
		atoms rvals = *attr;
		
		*ac = rvals.size();
		if (!(*av)) // otherwise use memory passed in
			*av = (max::t_atom*)max::sysmem_newptr(sizeof(max::t_atom) * *ac);
		for (auto i=0; i<*ac; ++i)
			(*av)[i] = rvals[i];
		
		return 0;
	}
	
	
	template<class T>
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, atom_reference args) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, ps_getname);
		auto& attr = self->obj.attributes[attr_name->s_name];
		*attr = atoms(args.begin(), args.end());
		return 0;
	}

	
	
	
	// maxname may come in as an entire path because of use of the __FILE__ macro
	std::string deduce_maxclassname(const char* maxname) {
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
			smaxname.assign(start, end-start);
		}
		return smaxname;
	}
	
}} // namespace c74::min
