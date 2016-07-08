/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	// All objects use A_GIMME signature for construction
	
	template<class T>
	max::t_object* min_new(max::t_symbol *name, atom_reference args) {
		long		attrstart = attr_args_offset(args.size(), args.begin());		// support normal arguments
		minwrap<T>*	self = (minwrap<T>*)max::object_alloc(this_class);
		
		self->obj.assign_instance((max::t_object*)self); // maxobj needs to be set prior to placement new
		new(&self->obj) T(atoms(args.begin(), args.begin()+attrstart)); // placement new
		self->obj.postinitialize();
		self->obj.set_classname(name);
        
		self->setup();
		
		auto outlets = self->obj.outlets();
		for (auto outlet = outlets.rbegin(); outlet != outlets.rend(); ++outlet) {
			if ((*outlet)->type == "")
				(*outlet)->instance = max::outlet_new(self, nullptr);
			else
				(*outlet)->instance = max::outlet_new(self, (*outlet)->type.c_str());
		}
		
		max::attr_args_process(self, args.size(), args.begin());
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
			const auto& outlet = self->obj.outlets()[a];
			strcpy(s, outlet->description.c_str());
		}
		else {
			const auto& inlet = self->obj.inlets()[a];
			strcpy(s, inlet->description.c_str());
		}
	}
	

   	template<class T>
    void min_bang(minwrap<T>* self) {
    	auto& meth = self->obj.methods()["bang"];
    	atoms as;
    	meth->function(as);
    }
    
	
	template<class T>
	void min_int(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods()["int"];
		atoms as = {v};
		meth->function(as);
	}

	
	template<class T>
	void min_toggle(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods()["toggle"];
		atoms as = {v};
		meth->function(as);
	}
	
	
	template<class T>
	void min_int_converted_to_float(minwrap<T>* self, long v) {
		auto& meth = self->obj.methods()["float"];
		atoms as = {v};
		meth->function(as);
	}
	
	
	template<class T>
	void min_float(minwrap<T>* self, double v) {
		auto& meth = self->obj.methods()["float"];
		atoms as = {v};
		meth->function(as);
	}

	
	template<class T>
	void min_method(minwrap<T>* self, max::t_symbol *s, atom_reference ar) {
		auto& meth = self->obj.methods()[s->s_name];
		atoms as(atoms(ar.begin(), ar.end()));
		meth->function(as);
	}

	
	template<class T>
	void min_method_dblclick(minwrap<T>* self) {
		auto& meth = self->obj.methods()["dblclick"];
		atoms as;
		meth->function(as);
	}

	template<class T>
	void min_method_patchlineupdate(minwrap<T>* self, max::t_object* patchline, long updatetype, max::t_object *src, long srcout, max::t_object *dst, long dstin) {
		auto& meth = self->obj.methods()["patchlineupdate"];
		atoms as(7);
		
		as[0] = self;
		as[1] = patchline;
		as[2] = updatetype;
		as[3] = src;
        as[4] = srcout;
        as[5] = dst;
        as[6] = dstin;
		meth->function(as);
	}
    
	template<class T>
	void min_method_notify(minwrap<T>* self, max::t_symbol*s, max::t_symbol* msg, void* sender, void* data) {
		auto& meth = self->obj.methods()["notify"];
		atoms as(5);
		
		as[0] = self;
		as[1] = s;
		as[2] = msg;
		as[3] = sender;
		as[4] = data;
		meth->function(as);
	}

	template<class T>
	void min_method_appendtodictionary(minwrap<T>* self, max::t_dictionary* d) {
		auto& meth = self->obj.methods()["savestate"];
		atoms as = {d};
		meth->function(as);
	}

	template<class T>
	void min_method_edclose(minwrap<T>* self) {
		auto& meth = self->obj.methods()["edclose"];
		atoms as;// = atoms_from_acav(ac, av);
		meth->function(as);
	}
	
	template<class T>
	void min_method_okclose(minwrap<T>* self) {
		auto& meth = self->obj.methods()["okclose"];
		atoms as;// = atoms_from_acav(ac, av);
		meth->function(as);
	}

	
	template<class T>
	void min_anything(minwrap<T>* self, max::t_symbol *s, atom_reference ar) {
		auto& meth = self->obj.methods()["anything"];
		atoms as(ar.begin(), ar.end());
		as.insert(as.begin(), atom(s));
		meth->function(as);
	}

	
	template<class T>
	void min_dictionary(minwrap<T>* self, max::t_symbol *s) {
		auto& meth = self->obj.methods()["dictionary"];
		auto d = dictobj_findregistered_retain(s);
		atoms as = { atom(d) };
		meth->function(as);
		if (d)
			dictobj_release(d);
	}
	
	
}} // namespace c74::min




template<class cpp_classname>
c74::max::t_class* define_min_external_common(cpp_classname& instance, const char* cppname, const char* cmaxname, void *resources) {
	std::string maxname = c74::min::deduce_maxclassname(cmaxname);
	
	c74::max::t_class* c = c74::max::class_new( maxname.c_str() ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : instance.methods()) {
		if (a_method.first == "dblclick")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_dblclick<cpp_classname>, "dblclick", c74::max::A_CANT, 0);
		else if (a_method.first == "patchlineupdate")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_patchlineupdate<cpp_classname>, "patchlineupdate", c74::max::A_CANT, 0);
		else if (a_method.first == "dspsetup")
			; // skip -- handle it in operator classes
		else if (a_method.first == "notify")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_notify<cpp_classname>, "notify", c74::max::A_CANT, 0);
		else if (a_method.first == "savestate")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_appendtodictionary<cpp_classname>, "appendtodictionary", c74::max::A_CANT, 0);
		else if (a_method.first == "okclose")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_okclose<cpp_classname>, "okclose", c74::max::A_CANT, 0);
		else if (a_method.first == "edclose")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method_edclose<cpp_classname>, "edclose", c74::max::A_CANT, 0);
		else if (a_method.first == "dictionary")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_dictionary<cpp_classname>, "dictionary", c74::max::A_SYM, 0);
		else if (a_method.first == "anything")
			class_addmethod(c, (c74::max::method)c74::min::min_anything<cpp_classname>, "anything", c74::max::A_GIMME, 0);
		else if (a_method.first == "bang")
			class_addmethod(c, (c74::max::method)c74::min::min_bang<cpp_classname>, "bang", c74::max::A_NOTHING, 0);
		else if (a_method.first == "toggle")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_toggle<cpp_classname>, "int", c74::max::A_LONG, 0);
		else if (a_method.first == "int")
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_int<cpp_classname>, "int", c74::max::A_LONG, 0);
		else if (a_method.first == "float") {
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_float<cpp_classname>, "float", c74::max::A_FLOAT, 0);
			
			// if there is a 'float' method but no 'int' method, generate a wrapper for it
			auto got = instance.methods().find("int");
			if ( got == instance.methods().end() )
				c74::max::class_addmethod(c, (c74::max::method)c74::min::min_int_converted_to_float<cpp_classname>, "int", c74::max::A_LONG, 0);
		}
		else if (a_method.first == "maxclass_setup")
			; // for min class construction only, do not add for exposure to max
		else
			c74::max::class_addmethod(c, (c74::max::method)c74::min::min_method<cpp_classname>, a_method.first.c_str(), a_method.second->type, 0);
	}
	
	
	for (auto& an_attribute : instance.attributes()) {
		std::string					attr_name = an_attribute.first;
		c74::min::attribute_base&	attr = *an_attribute.second;
		
		attr.create(c, (c74::max::method)c74::min::min_attr_getter<cpp_classname>, (c74::max::method)c74::min::min_attr_setter<cpp_classname>);
		
		// Attribute Metadata
		CLASS_ATTR_LABEL(c,	attr_name.c_str(), 0, attr.label_string());
		if (attr.range_string()) {
			if (attr.datatype() == "symbol")
				CLASS_ATTR_ENUM(c,	attr_name.c_str(), 0, attr.range_string());
		}
		
	}

	instance.try_call("maxclass_setup", c);
	return c;
}


template<class cpp_classname>
typename std::enable_if<
	!std::is_base_of<c74::min::sample_operator_base, cpp_classname>::value
&& 	!std::is_base_of<c74::min::perform_operator_base, cpp_classname>::value
>::type
define_min_external_audio(c74::max::t_class*) {}


template<class cpp_classname>
void define_min_external_finish(c74::max::t_class* c) {
	c74::max::class_addmethod(c, (c74::max::method)c74::min::min_assist<cpp_classname>, "assist", c74::max::A_CANT, 0);
	c74::max::class_register(c74::max::CLASS_BOX, c);
}


// Note: Jitter Matrix Operators are significantly different enough that they overload define_min_external()
// in c74_min_operator_matrix.h

template<class cpp_classname>
typename std::enable_if<
	!std::is_base_of<c74::min::matrix_operator, cpp_classname>::value
	&& !std::is_base_of<c74::min::gl_operator, cpp_classname>::value
>::type
define_min_external(const char* cppname, const char* maxname, void *resources, cpp_classname* instance = nullptr) {
	c74::min::this_class_init = true;

	std::unique_ptr<cpp_classname> dummy_instance = nullptr;

	if (!instance) {
		dummy_instance = std::make_unique<cpp_classname>();
		instance = dummy_instance.get();
	}	
	
	auto c = define_min_external_common<cpp_classname>(*instance, cppname, maxname, resources);
	//if (std::is_base_of<c74::min::audio_object, cpp_classname>())
	define_min_external_audio<cpp_classname>(c);
	define_min_external_finish<cpp_classname>(c);
	c74::min::this_class = c;
}








namespace c74 {
namespace min {

	// Finally, we define the min::object
	// Has to happen last because it includes code that does lazy initialization of the above class definitions
	// in the event that it hasn't occurred.

	/// We pass the class type as a template parameter to this class.
	/// This allows for [static polymorphism](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism).
	/// One benefits of this are leveraged when instantiating class instances directly instead of through the Max interface,
	/// such as when unit testing or embedding an object inside of another object.

	template <class T>
	class object : public object_base {
	public:
		object() {
			// The way objects are created for the Max environment requires that memory be allocated first
			// using object_alloc(), which is followed by the use of placement-new to contruct the C++ class.
			// When this occurs the m_maxobj member is already set prior to the constructor being run.
			// If there is no valid m_maxobj then that means this class was created outside of the Max environment.
			//
			// This could occur if a class uses another class directly or in the case of unit testing.
			// In such cases we need to do something reasonable so the our invariants can be held true.
			
			if (m_initializing) {							// we are being initialized externally via placement new
				;
			}
			else {											// we need to initialize ourselves
				if (!this_class_init) {						// if we aren't already in the process of initializing...
					std::string maxname = typeid(T).name();
					maxname += "_max";
					define_min_external<T> ( typeid(T).name(), maxname.c_str(), nullptr, (T*)this );
				}
				if (this_class) {
					m_maxobj = (max::t_object*)max::object_alloc(this_class);
					
					// TODO: assign???
					postinitialize();
					
					// self->setup();
					//
					// auto outlets = self->obj.outlets();
					// for (auto outlet = outlets.rbegin(); outlet != outlets.rend(); ++outlet) {
					//	if ((*outlet)->type == "")
					//		(*outlet)->instance = max::outlet_new(self, nullptr);
					//	else
					//		(*outlet)->instance = max::outlet_new(self, (*outlet)->type.c_str());
					//}
					//
					//max::attr_args_process(self, args.size(), args.begin());
				}
			}
		}
		
		virtual ~object() {}

		
	protected:
		logger	cout = { this, logger::type::message};
		logger	cerr = { this, logger::type::error};
	};

}} // namespace c74::min

