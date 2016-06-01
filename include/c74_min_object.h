/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
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
	

	
}} // namespace c74::min




template<class cpp_classname>
void define_min_external_common(const char* cppname, const char* cmaxname, void *resources) {
	c74::min::atoms	a;
	cpp_classname	dummy(a);
	std::string		maxname = c74::min::deduce_maxclassname(cmaxname);
	
	c74::min::this_class = c74::max::class_new( maxname.c_str() ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : dummy.methods) {
		if (a_method.first == "dblclick")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_dblclick<cpp_classname>, "dblclick", c74::max::A_CANT, 0);
		else if (a_method.first == "dspsetup")
			; // skip -- handle it in operator classes
		else if (a_method.first == "notify")
			c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_method_notify<cpp_classname>, "notify", c74::max::A_CANT, 0);
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
}


template<class cpp_classname>
typename std::enable_if<
	!std::is_base_of<c74::min::sample_operator_base, cpp_classname>::value
&& 	!std::is_base_of<c74::min::perform_operator_base, cpp_classname>::value
	>::type
define_min_external_audio() {}


template<class cpp_classname>
void define_min_external_finish() {
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::min::min_assist<cpp_classname>, "assist", c74::max::A_CANT, 0);
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}


// Note: Jitter Matrix Operators are significantly different enough that they overload define_min_external()
// in c74_min_operator_matrix.h

template<class cpp_classname>
typename std::enable_if<
	!std::is_base_of<c74::min::matrix_operator, cpp_classname>::value
	&& !std::is_base_of<c74::min::gl_operator, cpp_classname>::value
>::type
define_min_external(const char* cppname, const char* maxname, void *resources) {
	define_min_external_common<cpp_classname>(cppname, maxname, resources);
	//if (std::is_base_of<c74::min::audio_object, cpp_classname>())
	define_min_external_audio<cpp_classname>();
	define_min_external_finish<cpp_classname>();
}
