#pragma once

#include "c74_max.h"
#include "c74_msp.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

#include "c74_min_symbol.h"
#include "c74_min_atom.h"
#include "c74_min_dictionary.h"

namespace c74 {
namespace min {
	static max::t_class* this_class = nullptr;
	
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
			return proxy_getinlet((max::t_object*)obj);
		}

		max::t_object*										obj = nullptr;
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
	};
	
	
	class outlet  : public port {
	public:
		outlet(object_base* an_owner, std::string a_description, std::string a_type = "")
		: port(an_owner, a_description, a_type)
		{
			owner->outlets.push_back(this);
		}
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
			else if (a_name == "dsp64")
				type = max::A_CANT;
			owner->methods[a_name] = this;
		}
		
		void operator ()(atoms args) {
			function(args);
		}
		
		void operator ()() {
			atoms a;
			function(a);
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
			if (owner->obj)
				object_attr_touch(owner->obj, name);
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
		
		
	private:
		T				value;
	};
	
#ifdef __APPLE__
# pragma mark -
# pragma mark c wrap
#endif

	// Wrap the C++ class together with the appropriate Max object header
	// Max object header is selected automatically using the type of the base class.

	template<class T, class=void>
	struct minwrap;
	
	template<class T>
	struct minwrap < T, typename std::enable_if< std::is_base_of< min::object, T>::value >::type > {
		max::t_object 			header;
		T						obj;
		
		void cleanup() {}
	};
	
	template<class T>
	struct minwrap < T, typename std::enable_if< std::is_base_of< min::audio_object, T>::value >::type > {
		max::t_pxobject 		header;
		T						obj;
		
		void cleanup() {
			dsp_free(&header);
		}
	};
	
	
	// All objects use A_GIMME signature for construction
	
	template<class T>
	max::t_object* min_new(max::t_symbol *name, long argc, max::t_atom* argv) {
		long		attrstart = attr_args_offset(argc, argv);		// support normal arguments
		minwrap<T>*	self = (minwrap<T>*)max::object_alloc(this_class);
		
		new(&self->obj) T(atoms_from_acav(attrstart, argv)); // placement new
		self->obj.obj = (max::t_object*)self;
		
		max::dsp_setup((max::t_pxobject*)self, self->obj.inlets.size());
		for (auto& outlet : self->obj.outlets) {
			if (outlet->type == "")
				max::outlet_new(self, nullptr);
			else
				max::outlet_new(self, outlet->type.c_str());
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

	
	
	struct audio_bundle {
		double **samples;
		long channel_count;
		long frame_count;
	};
	
	template<class T>
	void min_perform(minwrap<T>* self, max::t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam) {
		audio_bundle input = {ins, numins, sampleframes};
		audio_bundle output = {outs, numouts, sampleframes};
		self->obj.perform(input, output);
	}
	
	
	
	
	template <typename T>
	struct has_dspsetup {
		template <class, class> class checker;
		
		template <typename C>
		static std::true_type test(checker<C, decltype(&C::dspsetup)> *);
		
		template <typename C>
		static std::false_type test(...);
		
		typedef decltype(test<T>(nullptr)) type;
		static const bool value = std::is_same<std::true_type, decltype(test<T>(nullptr))>::value;
	};
	
	//	static_assert(has_dspsetup<slide>::value, "error");
	

	template<class T>
	typename std::enable_if< has_dspsetup<T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		int i = 0;
		
		while (i < self->obj.inlets.size()) {
			self->obj.inlets[i]->signal_connection = count[i];
			++i;
		}
		while (i < self->obj.outlets.size()) {
			self->obj.outlets[i - self->obj.inlets.size()]->signal_connection = count[i];
			++i;
		}
	
		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->obj.dspsetup(args);
		
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_perform<T>, 0, NULL);
	}
	
	template<class T>
	typename std::enable_if< !has_dspsetup<T>::value>::type
	min_dsp64_sel(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		int i = 0;
		
		while (i < self->obj.inlets.size()) {
			self->obj.inlets[i]->signal_connection = count[i];
			++i;
		}
		while (i < self->obj.outlets.size()) {
			self->obj.outlets[i - self->obj.inlets.size()]->signal_connection = count[i];
			++i;
		}
		
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, max::gensym("dsp_add64"), (max::t_object*)self, (max::t_perfroutine64)min_perform<T>, 0, NULL);
	}

  template<class T>
	inline void min_dsp64(minwrap<T>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
    min_dsp64_sel<T>(self, dsp64, count, samplerate, maxvectorsize, flags);
  }
}} // namespace c74::min


template<class cpp_classname>
void define_min_external(const char* maxname, void *resources)
{
	c74::min::atoms	a;
	cpp_classname	dummy(a);
	
	c74::min::this_class = c74::max::class_new( maxname ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : dummy.methods) {
		if (a_method.first == "dspsetup")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
		else if (a_method.first == "bang")
			class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_bang<cpp_classname>, "bang", c74::max::A_NOTHING, 0);
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
	
	// if there is no custom dspsetup, and this is an audio_object, add the default
	if (std::is_base_of<c74::min::audio_object, cpp_classname>()) {
		auto got = dummy.methods.find("dspsetup");
		if ( got == dummy.methods.end() )
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_dsp64<cpp_classname>, "dsp64", c74::max::A_CANT, 0);
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
	
	c74::max::class_dspinit(c74::min::this_class);\
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}


#define MIN_EXTERNAL(cpp_classname,max_classname) \
void ext_main (void* r) { \
	define_min_external< cpp_classname > ( max_classname , r ); \
}

#define METHOD( name )						c74::min::method				name = { this, #name , [this](c74::min::atoms& args)
#define ATTRIBUTE( name, type, default )	c74::min::attribute< type >		name = { this, #name , default, [this](c74::min::atoms& args)
#define INLET(  name, desc )				c74::min::inlet					name = { this, desc };
#define OUTLET( name, ... )					c74::min::outlet				name = { this, __VA_ARGS__ };
#define END };
