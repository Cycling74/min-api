/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class inlet;
	class outlet;
	class method;
	class attribute_base;
	template <typename T> class attribute;
	
	
	class object_base {
	public:
		
		~object_base() {
			
		}

		int current_inlet() {
			return proxy_getinlet((max::t_object*)maxobj);
		}

		max::t_object*										maxobj;
		bool												initialized = false;
		std::vector<min::inlet*>							inlets;
		std::vector<min::outlet*>							outlets;
		std::unordered_map<std::string, method*>			methods;
		std::unordered_map<std::string, attribute_base*>	attributes;
	};
	
	
	class object : public object_base {
		;
	};

	
	class audio_object : public object_base {
		;
	};
	

	class matrix_object : public object_base {
		;
	};
	
	
	// Wrap the C++ class together with the appropriate Max object header
	// Max object header is selected automatically using the type of the base class.
	
	template<class T, class=void>
	struct minwrap;
	
	template<class T>
	struct minwrap < T, typename std::enable_if< std::is_base_of< min::object, T>::value >::type > {
		max::t_object 			header;
		T						obj;
		
		void setup() {
			auto self = &header;
			//max::dsp_setup((max::t_pxobject*)self, self->obj.inlets.size());
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
		max::t_symbol*		name;
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
	
#ifdef __APPLE__
# pragma mark -
# pragma mark c wrap
#endif

	
	// All objects use A_GIMME signature for construction
	
	template<class T>
	max::t_object* min_new(max::t_symbol *name, long argc, max::t_atom* argv) {
		long		attrstart = attr_args_offset(argc, argv);		// support normal arguments
		minwrap<T>*	self = (minwrap<T>*)max::object_alloc(this_class);
		
		self->obj.maxobj = (max::t_object*)self; // maxobj needs to be set prior to placement new
		new(&self->obj) T(atoms_from_acav(attrstart, argv)); // placement new
		self->obj.initialized = true;
		
		self->setup();
		
		for (auto outlet = self->obj.outlets.rbegin(); outlet != self->obj.outlets.rend(); ++outlet) {
			if ((*outlet)->type == "")
				(*outlet)->instance = max::outlet_new(self, nullptr);
			else
				(*outlet)->instance = max::outlet_new(self, (*outlet)->type.c_str());
		}
		
		max::attr_args_process(self, argc, argv);
		
		return (max::t_object*)self;
	}
	
	
	template<class T>
	void min_free(minwrap<T>* self) {
		self->cleanup();
		self->obj.~T(); // placement delete
	}
	
	
	template<class T>
	void min_assist(minwrap<T>* self, void *b, long m, long a, char *s) {
		if (m == 2) {
			const auto& outlet = self->obj.outlets[a];
			strcpy(s, outlet->description.c_str());
		}
		else {
			const auto& inlet = self->obj.inlets[a];
			strcpy(s, inlet->description.c_str());
		}
	}
	

   	template<class T>
    void min_bang(minwrap<T>* self) {
    	auto& meth = self->obj.methods["bang"];
    	atoms as;
    	meth->function(as);
    }
    
	
	template<class T>
	void min_int(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods["int"];
		max::t_atom a;
		atom_setlong(&a, v);
		atoms as = atoms_from_acav(1, &a);
		meth->function(as);
	}

	
	template<class T>
	void min_toggle(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods["toggle"];
		max::t_atom a;
		atom_setlong(&a, v);
		atoms as = atoms_from_acav(1, &a);
		meth->function(as);
	}
	
	
	template<class T>
	void min_int_converted_to_float(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods["float"];
		atoms a;
		a.push_back(v);
		meth->function(a);
	}
	
	
	template<class T>
	void min_float(minwrap<T>* self, double v) {
		auto& meth = self->obj.methods["float"];
		atoms a;
		a.push_back(v);
		meth->function(a);
	}

	
	template<class T>
	void min_method(minwrap<T>* self, max::t_symbol *s, long ac, max::t_atom *av) {
		auto& meth = self->obj.methods[s->s_name];
		atoms as = atoms_from_acav(ac, av);
		meth->function(as);
	}

	
	template<class T>
	void min_method_dblclick(minwrap<T>* self) {
		auto& meth = self->obj.methods["dblclick"];
		atoms as;// = atoms_from_acav(ac, av);
		meth->function(as);
	}

	template<class T>
	void min_method_notify(minwrap<T>* self, max::t_symbol*s, max::t_symbol* msg, void* sender, void* data) {
		auto& meth = self->obj.methods["notify"];
		atoms as(5);
		
		as[0] = self;
		as[1] = s;
		as[2] = msg;
		as[3] = sender;
		as[4] = data;
		meth->function(as);
	}
	
	template<class T>
	void min_method_edclose(minwrap<T>* self) {
		auto& meth = self->obj.methods["edclose"];
		atoms as;// = atoms_from_acav(ac, av);
		meth->function(as);
	}
	
	template<class T>
	void min_method_okclose(minwrap<T>* self) {
		auto& meth = self->obj.methods["okclose"];
		atoms as;// = atoms_from_acav(ac, av);
		meth->function(as);
	}

	
	template<class T>
	void min_anything(minwrap<T>* self, max::t_symbol *s, long ac, max::t_atom *av) {
		auto& meth = self->obj.methods["anything"];
		atoms as = atoms_from_acav(ac, av);
		as.insert(as.begin(), atom(s));
		meth->function(as);
	}

	
	template<class T>
	void min_dictionary(minwrap<T>* self, max::t_symbol *s) {
		auto& meth = self->obj.methods["dictionary"];
		auto d = dictobj_findregistered_retain(s);
		atoms as = { atom(d) };
		meth->function(as);
		dictobj_release(d);
	}
	
	
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
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, long ac, max::t_atom* av) {
		max::t_symbol* attr_name = (max::t_symbol*)max::object_method(maxattr, ps_getname);
		auto& attr = self->obj.attributes[attr_name->s_name];
		*attr = atoms_from_acav(ac, av);
		return 0;
	}

	
}} // namespace c74::min



template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::object, cpp_classname>::value>::type
define_min_external(const char* cppname, const char* maxname, void *resources)
{
	c74::min::atoms	a;
	cpp_classname	dummy(a);
	std::string		smaxname;
	
	// maxname may come in as an entire path because of use of the __FILE__ macro
	{
		const char* start = strrchr(maxname, '/');
		if (start)
			start += 1;
		else
			start = maxname;
		
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
	}
	
	c74::min::this_class = c74::max::class_new( smaxname.c_str() ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : dummy.methods) {
		if (a_method.first == "dblclick")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_dblclick<cpp_classname>, "dblclick", c74::max::A_CANT, 0);
		else if (a_method.first == "okclose")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_okclose<cpp_classname>, "okclose", c74::max::A_CANT, 0);
		else if (a_method.first == "edclose")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_edclose<cpp_classname>, "edclose", c74::max::A_CANT, 0);
		else if (a_method.first == "dictionary")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dictionary<cpp_classname>, "dictionary", c74::max::A_SYM, 0);
		else if (a_method.first == "anything")
			class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_anything<cpp_classname>, "anything", c74::max::A_GIMME, 0);
		else if (a_method.first == "bang")
			class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_bang<cpp_classname>, "bang", c74::max::A_NOTHING, 0);
		else if (a_method.first == "toggle")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_toggle<cpp_classname>, "int", c74::max::A_LONG, 0);
		else if (a_method.first == "int")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_int<cpp_classname>, "int", c74::max::A_LONG, 0);
		else if (a_method.first == "float") {
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_float<cpp_classname>, "float", c74::max::A_FLOAT, 0);
			
			// if there is a 'float' method but no 'int' method, generate a wrapper for it
			auto got = dummy.methods.find("int");
			if ( got == dummy.methods.end() )
				c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_int_converted_to_float<cpp_classname>, "int", c74::max::A_LONG, 0);
		}
		else
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method<cpp_classname>, a_method.first.c_str(), a_method.second->type, 0);
	}
	
	for (auto& an_attribute : dummy.attributes) {
		std::string				attr_name = an_attribute.first;
		//min::attribute_base*	attr = an_attribute.second;
		
		c74::max::class_addattr(c74::min::this_class,
								c74::max::attr_offset_new(attr_name.c_str(),
														  c74::max::gensym("float64"),
														  0,
														  (c74::max::method)c74::min::min_attr_getter<cpp_classname>,
														  (c74::max::method)c74::min::min_attr_setter<cpp_classname>,
														  0
														  )
								);
	}
	
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_assist<cpp_classname>, "assist", c74::max::A_CANT, 0);
	
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}


