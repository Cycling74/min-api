/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	// All objects use A_GIMME signature for construction
	// However, all <in classes may not define a constructor to handle those arguments.

	template<class min_class_type>
	typename std::enable_if< std::is_constructible<min_class_type,atoms>::value >::type
	min_ctor(minwrap<min_class_type>* self, const atoms& args) {
		new(&self->min_object) min_class_type(args); // placement new
	}
	
	template<class min_class_type>
	typename std::enable_if< !std::is_constructible<min_class_type,atoms>::value >::type
	min_ctor(minwrap<min_class_type>* self, const atoms& args) {
		new(&self->min_object) min_class_type; // placement new
	}
	
	
	template<class min_class_type>
	max::t_object* wrapper_new(max::t_symbol* name, long ac, max::t_atom* av) {
		atom_reference	args(ac, av);
		long	attrstart = attr_args_offset(args.size(), args.begin());		// support normal arguments
		auto	self = (minwrap<min_class_type>*)max::object_alloc(this_class);

		self->min_object.assign_instance((max::t_object*)self); // maxobj needs to be set prior to placement new
		min_ctor<min_class_type>(self, atoms(args.begin(), args.begin()+attrstart));
		self->min_object.postinitialize();
		self->min_object.set_classname(name);
        
		self->setup();
				
		max::attr_args_process(self, args.size(), args.begin());
		return (max::t_object*)self;
	}


	template<class min_class_type>
	void wrapper_free(minwrap<min_class_type>* self) {
		self->cleanup();		// cleanup routine specific to each type of object (e.g. to call dsp_free() for audio objects)
		self->min_object.~min_class_type();	// placement delete
	}


	template<class min_class_type>
	void wrapper_method_assist(minwrap<min_class_type>* self, void *b, long m, long a, char *s) {
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
	template<class min_class_type>
	void wrapper_method_generic(minwrap<min_class_type>* self, max::t_symbol *s, long ac, max::t_atom* av) {
		auto& meth = *self->min_object.methods()[s->s_name];
		atoms as(ac);
		for (auto i=0; i<ac; ++i)
			as[i] = av[i];
		meth(as);
	}
	



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
		c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_##wrappermethod<min_class_type,wrapper_method_name_##methname>, wrapper_method_name_##methname, c74::max::methtype, 0);


	template<class min_class_type>
	c74::max::t_class* wrap_as_max_external_common(min_class_type& instance, const char* cppname, const char* cmaxname, void *resources) {
		std::string maxname = c74::min::deduce_maxclassname(cmaxname);
		
		auto* c = c74::max::class_new( maxname.c_str() ,(c74::max::method)c74::min::wrapper_new<min_class_type>, (c74::max::method)c74::min::wrapper_free<min_class_type>, sizeof( c74::min::minwrap<min_class_type> ), nullptr, c74::max::A_GIMME, 0);
		
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
				c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_generic<min_class_type>, a_method.first.c_str(), a_method.second->type(), 0);
			
			// if there is a 'float' method but no 'int' method, generate a wrapper for it
			if (a_method.first == "float" && (instance.methods().find("int") == instance.methods().end()))
				c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_int<min_class_type,wrapper_method_name_float>, "int", c74::max::A_LONG, 0);
		}
		
		
		for (auto& an_attribute : instance.attributes()) {
			std::string					attr_name = an_attribute.first;
			c74::min::attribute_base&	attr = *an_attribute.second;
			
			attr.create(c, (c74::max::method)c74::min::min_attr_getter<min_class_type>, (c74::max::method)c74::min::min_attr_setter<min_class_type>);
			
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


	template<class min_class_type>
	typename std::enable_if<
		!std::is_base_of<c74::min::sample_operator_base, min_class_type>::value
	&& 	!std::is_base_of<c74::min::perform_operator_base, min_class_type>::value
	>::type
	wrap_as_max_external_audio(c74::max::t_class*) {}


	template<class min_class_type>
	void wrap_as_max_external_finish(c74::max::t_class* c) {
		c74::max::class_addmethod(c, (c74::max::method)c74::min::wrapper_method_assist<min_class_type>, "assist", c74::max::A_CANT, 0);
		c74::max::class_register(c74::max::CLASS_BOX, c);
	}


	// Note: Jitter Matrix Operators are significantly different enough that they overload define_min_external()
	// in c74_min_operator_matrix.h

	template<class min_class_type>
	typename std::enable_if<
		!std::is_base_of<c74::min::matrix_operator, min_class_type>::value
		&& !std::is_base_of<c74::min::gl_operator, min_class_type>::value
	>::type
	wrap_as_max_external(const char* cppname, const char* maxname, void *resources, min_class_type* instance = nullptr) {
		c74::min::this_class_init = true;

		std::unique_ptr<min_class_type> dummy_instance = nullptr;

		if (!instance) {
			dummy_instance = std::make_unique<min_class_type>();
			instance = dummy_instance.get();
		}	
		
		auto c = wrap_as_max_external_common<min_class_type>(*instance, cppname, maxname, resources);
		//if (std::is_base_of<c74::min::audio_object, cpp_classname>())
		wrap_as_max_external_audio<min_class_type>(c);
		wrap_as_max_external_finish<min_class_type>(c);
		c74::min::this_class = c;
		instance->try_call("maxclass_setup", c);
	}
	
	
}} // namespace c74::min

