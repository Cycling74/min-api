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
	
	
	// Return the pointer to 'self' ... that is to say the pointer to the instance of our Max class
	// In the Jitter case there is a version of this function that returns instead the instance of the Jitter class
	template<class min_class_type, typename std::enable_if< !std::is_base_of<c74::min::matrix_operator, min_class_type>::value, int>::type = 0>
	minwrap<min_class_type>* wrapper_find_self(max::t_object* obj) {
		return (minwrap<min_class_type>*)obj;
	}
	
	template<class min_class_type, typename std::enable_if< std::is_base_of<c74::min::matrix_operator, min_class_type>::value, int>::type = 0>
	minwrap<min_class_type>* wrapper_find_self(max::t_object* mob) {
		return (minwrap<min_class_type>*)max::max_jit_obex_jitob_get(mob);
	}

	
	
	template<class min_class_type, class method_name_type>
	void wrapper_method_zero(max::t_object* o) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		
		meth();
	}
	
	template<class min_class_type, class method_name_type>
	void wrapper_method_int(max::t_object* o, long v) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as = {v};
		
		meth(as);
	}

	template<class min_class_type, class method_name_type>
	void wrapper_method_float(max::t_object* o, double v) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as = {v};
		
		meth(as);
	}
	
	template<class min_class_type, class method_name_type>
	void wrapper_method_symbol(max::t_object* o, max::t_symbol* v) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as = {symbol(v)};
		
		meth(as);
	}

	template<class min_class_type, class method_name_type>
	void wrapper_method_anything(max::t_object* o, max::t_symbol *s, long ac, max::t_atom* av) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as(ac+1);
		
		as[0] = s;
		for (auto i=0; i<ac; ++i)
			as[i+1] = av[i];
		meth(as);
	}
	
	template<class min_class_type, class method_name_type>
	void wrapper_method_ptr(max::t_object* o, void* v) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as = {v};
		
		meth(as);
	}
	
	template<class min_class_type, class method_name_type>
	max::t_max_err wrapper_method_self_sym_sym_ptr_ptr___err(max::t_object* o, max::t_symbol* s1, max::t_symbol* s2, void* p1, void* p2) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as { o, s1, s2, p1, p2 };	// NOTE: self could be the jitter object rather than the max object -- so we pass `o` which is always the correct `self` for box operations
		
		return (long)meth(as).at(0);
	}
	
	template<class min_class_type, class method_name_type>
	void wrapper_method_self_ptr_long_ptr_long_ptr_long(max::t_object* o, void* arg1, long arg2, void* arg3, long arg4, void* arg5, long arg6) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		atoms	as { o, arg1, arg2, arg3, arg4, arg5, arg6 };	// NOTE: self could be the jitter object rather than the max object -- so we pass `o` which is always the correct `self` for box operations
		
		meth(as);
	}
	
	// dictionary is a very special case because of the reference counting
	template<class min_class_type, class method_name_type>
	void wrapper_method_dictionary(max::t_object* o, max::t_symbol *s) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[method_name_type::name];
		auto	d = dictobj_findregistered_retain(s);
		atoms	as = { atom(d) };
		
		meth(as);
		if (d)
			dictobj_release(d);
	}
	
	// this version is called for most min::method instances defined in the min class
	template<class min_class_type>
	void wrapper_method_generic(max::t_object* o, max::t_symbol *s, long ac, max::t_atom* av) {
		auto	self = wrapper_find_self<min_class_type>(o);
		auto&	meth = *self->min_object.methods()[s->s_name];
		atoms	as(ac);
		
		for (auto i=0; i<ac; ++i)
			as[i] = av[i];
		meth(as);
	}
	

	
	
	// In the above wrapper methods we need access to the Max method name,
	// either to pass it on or to perform a lookup.
	// However, we can't change the method function prototype so we instead
	// pass the name as a template parameter...
	// Except that you can pass non-integral data as a template parameter.
	// Thus we create types for each string that we would like to pass and then
	// specialize the template calls with the type.
	
	#define MIN_WRAPPER_CREATE_TYPE_FROM_STRING(str)\
	struct wrapper_method_name_##str { static const char name[]; };\
	const char wrapper_method_name_##str::name[] = #str;
	
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(anything)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(appendtodictionary)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(bang)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(dblclick)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(dictionary)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(edclose)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(float)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(int)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(notify)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(okclose)
	MIN_WRAPPER_CREATE_TYPE_FROM_STRING(patchlineupdate)

	
	// Simplify the meth switches in the following code to reduce excessive and tedious code duplication

	#define MIN_WRAPPER_ADDMETHOD(c,methname,wrappermethod,methtype) \
	if (a_method.first == #methname) \
		c74::max::class_addmethod(c, (c74::max::method)wrapper_method_##wrappermethod<min_class_type,wrapper_method_name_##methname>, #methname, c74::max::methtype, 0);

	
	// Shared class definition code for wrapping a Min class as a Max class

	template<class min_class_type>
	c74::max::t_class* wrap_as_max_external_common(min_class_type& instance, const char* cppname, const char* cmaxname, void *resources) {
		std::string maxname = deduce_maxclassname(cmaxname);
		
		auto* c = c74::max::class_new(maxname.c_str() ,(c74::max::method)wrapper_new<min_class_type>, (c74::max::method)wrapper_free<min_class_type>, sizeof(minwrap<min_class_type>), nullptr, c74::max::A_GIMME, 0);
		
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
				c74::max::class_addmethod(c, (c74::max::method)wrapper_method_generic<min_class_type>, a_method.first.c_str(), a_method.second->type(), 0);
			
			// if there is a 'float' method but no 'int' method, generate a wrapper for it
			if (a_method.first == "float" && (instance.methods().find("int") == instance.methods().end()))
				c74::max::class_addmethod(c, (c74::max::method)wrapper_method_int<min_class_type,wrapper_method_name_float>, "int", c74::max::A_LONG, 0);
		}
		
		for (auto& an_attribute : instance.attributes()) {
			std::string		attr_name = an_attribute.first;
			attribute_base&	attr = *an_attribute.second;
			
			attr.create(c, (c74::max::method)min_attr_getter<min_class_type>, (c74::max::method)min_attr_setter<min_class_type>);
			
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
		c74::max::class_addmethod(c, (c74::max::method)wrapper_method_assist<min_class_type>, "assist", c74::max::A_CANT, 0);
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
	
	
	template<class min_class_type>
	typename std::enable_if<
	std::is_base_of<c74::min::matrix_operator, min_class_type>::value
	>::type
	wrap_as_max_external(const char* cppname, const char* cmaxname, void *resources, min_class_type* instance = nullptr) {
		using c74::max::method;
		using c74::max::class_addmethod;
		
		this_class_init = true;
		
		std::string						maxname = deduce_maxclassname(cmaxname);
		std::unique_ptr<min_class_type>	dummy_instance = nullptr;
		
		if (!instance) {
			dummy_instance = std::make_unique<min_class_type>();
			instance = dummy_instance.get();
		}
		
		// 1. Boxless Jit Class
		
		this_jit_class = (c74::max::t_class*)c74::max::jit_class_new(cppname, (method)jit_new<min_class_type>, (method)jit_free<min_class_type>, sizeof(minwrap<min_class_type>), 0);
		
		size_t inletcount = 0;
		for (auto i: instance->inlets()) {
			if (i->type() == "matrix")
				inletcount++;
		}
		
		size_t outletcount = 0;
		for (auto i: instance->outlets()) {
			if (i->type() == "matrix")
				outletcount++;
		}
		
		// If no matrix inputs are declared, the mop is a generator
		bool ownsinput = (inletcount==0);
		
		//add mop
		auto mop = c74::max::jit_object_new(c74::max::_jit_sym_jit_mop, inletcount, outletcount);
		c74::max::jit_class_addadornment(this_jit_class, mop);
		
		//add methods
		c74::max::jit_class_addmethod(this_jit_class, (method)jit_matrix_calc<min_class_type>, "matrix_calc", c74::max::A_CANT, 0);
		
		//add attributes
		long attrflags = c74::max::ATTR_GET_DEFER_LOW | c74::max::ATTR_SET_USURP_LOW;
		
		for (auto& an_attribute : instance->attributes()) {
			std::string		attr_name = an_attribute.first;
			attribute_base&	attr = *an_attribute.second;
			auto			jit_attr = c74::max::jit_object_new(c74::max::_jit_sym_jit_attr_offset, attr_name.c_str(), (c74::max::t_symbol*)attr.datatype(), attrflags, (method)min_attr_getter<min_class_type>, (method)min_attr_setter<min_class_type>, 0);

			c74::max::jit_class_addattr(this_jit_class, jit_attr);
			CLASS_ATTR_LABEL(this_jit_class, attr_name.c_str(), 0, attr.label_string());
			
			auto range_string = attr.range_string();
			if (!range_string.empty()) {
				if (attr.datatype() == "symbol")
					CLASS_ATTR_ENUM(this_jit_class, attr_name.c_str(), 0, range_string.c_str());
			}
		}
		
		jit_class_register(this_jit_class);
		instance->try_call("jitclass_setup", this_jit_class);
	
		
		// 2. Max Wrapper Class

		c74::max::t_class* c = c74::max::class_new(
												   maxname.c_str(),
												   (method)max_jit_mop_new<min_class_type>,
												   (method)max_jit_mop_free<min_class_type>,
												   sizeof(max_jit_wrapper),
												   nullptr,
												   c74::max::A_GIMME,
												   0
												   );
		
		c74::max::max_jit_class_obex_setup(c, calcoffset(max_jit_wrapper, obex));
		
		// for generator mops, override jit_matrix and outputmatrix
		long flags = (ownsinput ? c74::max::MAX_JIT_MOP_FLAGS_OWN_OUTPUTMATRIX|c74::max::MAX_JIT_MOP_FLAGS_OWN_JIT_MATRIX : 0);
		
		c74::max::max_jit_class_mop_wrap(c, c74::min::this_jit_class, flags);	// attrs & methods for name, type, dim, planecount, bang, outputmatrix, etc
		c74::max::max_jit_class_wrap_standard(c, c74::min::this_jit_class, 0);		// attrs & methods for getattributes, dumpout, maxjitclassaddmethods, etc
		
		c74::max::class_addmethod(c, (method)c74::max::max_jit_mop_assist, "assist", c74::max::A_CANT, 0);	// standard matrix-operator (mop) assist fn
		
		if(ownsinput)
			c74::max::max_jit_class_addmethod_usurp_low(c, (method)c74::min::min_jit_mop_outputmatrix<min_class_type>, (char*)"outputmatrix");
		
		for (auto& a_method : instance->methods()) {
			MIN_WRAPPER_ADDMETHOD(c, dictionary,			dictionary,							A_SYM)
			else MIN_WRAPPER_ADDMETHOD(c, notify,			self_sym_sym_ptr_ptr___err,			A_CANT)
			else MIN_WRAPPER_ADDMETHOD(c, patchlineupdate,	self_ptr_long_ptr_long_ptr_long,	A_CANT)
			else if (a_method.first == "maxclass_setup")	; // for min class construction only, do not add for exposure to max
			else
				class_addmethod(c, (method)wrapper_method_generic<min_class_type>, a_method.first.c_str(), a_method.second->type(), 0);
		}
		
		// the menufun isn't used anymore, so we are repurposing it here to store the name of the jitter class we wrap
		c->c_menufun = (c74::max::method)c74::max::gensym(cppname);
		
		c74::max::class_register(c74::max::CLASS_BOX, c);
		
		c74::min::this_class = c;
		instance->try_call("maxclass_setup", c);
	}
	
	#undef MIN_WRAPPER_ADDMETHOD
	#undef MIN_WRAPPER_CREATE_TYPE_FROM_STRING
	
}} // namespace c74::min
