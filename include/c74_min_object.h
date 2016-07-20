/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	// All objects use A_GIMME signature for construction
	// However, all <in classes may not define a constructor to handle those arguments.

	template<class T>
	typename std::enable_if< std::is_constructible<T,atoms>::value >::type
	min_ctor(minwrap<T>* self, const atoms& args) {
		new(&self->min_object) T(args); // placement new
	}
	
	template<class T>
	typename std::enable_if< !std::is_constructible<T,atoms>::value >::type
	min_ctor(minwrap<T>* self, const atoms& args) {
		new(&self->min_object) T; // placement new
	}
	
	
	template<class T>
	max::t_object* min_new(max::t_symbol* name, long ac, max::t_atom* av) {
		atom_reference	args(ac, av);
		long			attrstart = attr_args_offset(args.size(), args.begin());		// support normal arguments
		minwrap<T>*		self = (minwrap<T>*)max::object_alloc(this_class);

		self->min_object.assign_instance((max::t_object*)self); // maxobj needs to be set prior to placement new
		min_ctor<T>(self, atoms(args.begin(), args.begin()+attrstart));
		self->min_object.postinitialize();
		self->min_object.set_classname(name);
        
		self->setup();
				
		max::attr_args_process(self, args.size(), args.begin());
		return (max::t_object*)self;
	}


	template<class T>
	void min_free(minwrap<T>* self) {
		self->cleanup();		// cleanup routine specific to each type of object (e.g. to call dsp_free() for audio objects)
		self->min_object.~T();	// placement delete
	}


	template<class T>
	void min_assist(minwrap<T>* self, void *b, long m, long a, char *s) {
		if (m == 2) {
			const auto& outlet = self->min_object.outlets()[a];
			strcpy(s, outlet->description().c_str());
		}
		else {
			const auto& inlet = self->min_object.inlets()[a];
			strcpy(s, inlet->description().c_str());
		}
	}
	
	
	
	
	
	
	template<class min_class_type, const char* method_name>
	void wrapper_method_zero(minwrap<min_class_type>* self) {
		auto& meth = *self->min_object.methods()[method_name];
		meth();
	}
	
	template<class min_class_type, const char* method_name>
	void wrapper_method_int(minwrap<min_class_type>* self, long v) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as = {v};
		meth(as);
	}

	template<class min_class_type, const char* method_name>
	void wrapper_method_float(minwrap<min_class_type>* self, double v) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as = {v};
		meth(as);
	}
	
	template<class min_class_type, const char* method_name>
	void wrapper_method_symbol(minwrap<min_class_type>* self, max::t_symbol* v) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as = {symbol(v)};
		meth(as);
	}

	template<class min_class_type, const char* method_name>
	void wrapper_method_anything(minwrap<min_class_type>* self, max::t_symbol *s, long ac, max::t_atom* av) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as(ac+1);
		as[0] = s;
		for (auto i=0; i<ac; ++i)
			as[i+1] = av[i];
		meth(as);
	}
	
	template<class min_class_type, const char* method_name>
	void wrapper_method_ptr(minwrap<min_class_type>* self, void* v) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as = {v};
		meth(as);
	}
	
	template<class min_class_type, const char* method_name>
	max::t_max_err wrapper_method_self_sym_sym_ptr_ptr___err(minwrap<min_class_type>* self, max::t_symbol* s1, max::t_symbol* s2, void* p1, void* p2) {
		auto& meth = *self->min_object.methods()[method_name];
		atoms as { self, s1, s2, p1, p2 };
		return (long)meth(as).at(0);
	}
	
	template<class min_class_type, const char* method_name>
	void wrapper_method_self_ptr_long_ptr_long_ptr_long(minwrap<min_class_type>* self, void* arg1, long arg2, void* arg3, long arg4, void* arg5, long arg6) {
		auto& meth = *self->min_object.methods()["patchlineupdate"];
		atoms as { self, arg1, arg2, arg3, arg4, arg5, arg6 };
		meth(as);
	}
	
	// dictionary is a very special case because of the reference counting
	template<class min_class_type, const char* method_name>
	void wrapper_method_dictionary(minwrap<min_class_type>* self, max::t_symbol *s) {
		auto&	meth = *self->min_object.methods()[method_name];
		auto	d = dictobj_findregistered_retain(s);
		atoms	as = { atom(d) };
		meth(as);
		if (d)
			dictobj_release(d);
	}
	
	// this version is called for most min::method instances defined in the min class
	template<class T>
	void wrapper_method_generic(minwrap<T>* self, max::t_symbol *s, long ac, max::t_atom* av) {
		auto& meth = *self->min_object.methods()[s->s_name];
		atoms as(ac);
		for (auto i=0; i<ac; ++i)
			as[i] = av[i];
		meth(as);
	}
	
}} // namespace c74::min



static const char wrapper_method_name_anything[]			= "anything";
static const char wrapper_method_name_appendtodictionary[]	= "appendtodictionary";
static const char wrapper_method_name_bang[]				= "bang";
static const char wrapper_method_name_dblclick[]			= "dblclick";
static const char wrapper_method_name_dictionary[]			= "dictionary";
static const char wrapper_method_name_edclose[]				= "edclose";
static const char wrapper_method_name_float[]				= "float";
static const char wrapper_method_name_int[]					= "int";
static const char wrapper_method_name_notify[]				= "notify";
static const char wrapper_method_name_okclose[]				= "okclose";
static const char wrapper_method_name_patchlineupdate[]		= "patchlineupdate";


#define MIN_WRAPPER_ADDMETHOD(c,methname,wrappermethod,methtype) \
if (a_method.first == #methname) \
	c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_##wrappermethod<cpp_classname,wrapper_method_name_##methname>, wrapper_method_name_##methname, c74::max::methtype, 0);


template<class cpp_classname>
c74::max::t_class* define_min_external_common(cpp_classname& instance, const char* cppname, const char* cmaxname, void *resources) {
	std::string maxname = c74::min::deduce_maxclassname(cmaxname);
	
	c74::max::t_class* c = c74::max::class_new( maxname.c_str() ,(c74::max::method)c74::min::min_new<cpp_classname>, (c74::max::method)c74::min::min_free<cpp_classname>, sizeof( c74::min::minwrap<cpp_classname> ), nullptr, c74::max::A_GIMME, 0);
	
	for (auto& a_method : instance.methods()) {
		     MIN_WRAPPER_ADDMETHOD(c, bang,					zero,								A_NOTHING)
		else MIN_WRAPPER_ADDMETHOD(c, dblclick,				zero,								A_CANT)
		else MIN_WRAPPER_ADDMETHOD(c, okclose,				zero,								A_CANT)
		else MIN_WRAPPER_ADDMETHOD(c, edclose,				zero,								A_CANT)
		else MIN_WRAPPER_ADDMETHOD(c, anything,				anything,							A_GIMME)
		else MIN_WRAPPER_ADDMETHOD(c, int,					int,								A_LONG)
		else MIN_WRAPPER_ADDMETHOD(c, float,				float,								A_FLOAT)
		else MIN_WRAPPER_ADDMETHOD(c, dictionary,			dictionary,							A_SYM)
		else MIN_WRAPPER_ADDMETHOD(c, appendtodictionary,	ptr,								A_CANT)
		else MIN_WRAPPER_ADDMETHOD(c, notify,				self_sym_sym_ptr_ptr___err,			A_CANT)
		else MIN_WRAPPER_ADDMETHOD(c, patchlineupdate,		self_ptr_long_ptr_long_ptr_long,	A_CANT)
		else if (a_method.first == "dspsetup")				; // skip -- handle it in operator classes
		else if (a_method.first == "maxclass_setup")		; // for min class construction only, do not add for exposure to max
		else
			c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_generic<cpp_classname>, a_method.first.c_str(), a_method.second->type(), 0);
		
		// if there is a 'float' method but no 'int' method, generate a wrapper for it
		if (a_method.first == "float" && (instance.methods().find("int") == instance.methods().end()))
			c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_int<cpp_classname,wrapper_method_name_float>, "int", c74::max::A_LONG, 0);
	}
	
	
	for (auto& an_attribute : instance.attributes()) {
		std::string					attr_name = an_attribute.first;
		c74::min::attribute_base&	attr = *an_attribute.second;
		
		attr.create(c, (c74::max::method)c74::min::min_attr_getter<cpp_classname>, (c74::max::method)c74::min::min_attr_setter<cpp_classname>);
		
		// Attribute Metadata
		CLASS_ATTR_LABEL(c,	attr_name.c_str(), 0, attr.label_string());

		auto range_string = attr.range_string();
		if (!range_string.empty()) {
			if (attr.datatype() == "symbol")
				CLASS_ATTR_ENUM(c, attr_name.c_str(), 0, range_string.c_str());
		}
	}

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
	instance->try_call("maxclass_setup", c);
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
					postinitialize();
				}
			}			
		}
		
		virtual ~object() {}

		
	protected:
		logger	cout { this, logger::type::message };
		logger	cerr { this, logger::type::error };
	};

}} // namespace c74::min

