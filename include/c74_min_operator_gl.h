/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_jitter.h"

namespace c74 {
namespace min {

	class gl_operator_base {};

	class gl_operator : public gl_operator_base {};

	// nobox jitter object
	template<class min_class_type>
	struct minwrap <min_class_type, typename enable_if< is_base_of<gl_operator_base,min_class_type>::value >::type > {
		maxobject_base	max_base;
		min_class_type	min_object;
		
		void setup(atoms args) {
			symbol dest_name = args[0];
			jit_ob3d_new(max_base, dest_name);	// create and attach ob3d.
			min_object.create_outlets();
		}
		
		void cleanup() {
			max::jit_ob3d_free(max_base);
		}
	};

	
	
	
	using gl_ob = void;
	enum gl_drawing_primitive {
		tri_strip,
		triangles,
		quads,
		quad_grid
	};
	
	
	
	class glchunk {
	public:
		glchunk(gl_ob* context, gl_drawing_primitive prim, int planecount, int vertexcount, int indexcount)
		: ob3d { context }
		{
			// TODO: need a more elegant map
			symbol s;
			if (prim == tri_strip)
				s = "tri_strip";
			else if (prim == triangles)
				s = "triangles";
			else if (prim == quads)
				s = "quads";
			else // if (prim == squad_grid)
				s = "quad_grid";
			
			chunk = max::jit_glchunk_new(s, planecount, vertexcount, indexcount);
		}
		
		~glchunk() {
			jit_glchunk_delete(chunk);
		}
		
		void draw() {
			// draw our chunk of OpenGL geometry.
			// if the ob3d is in matrixoutput mode, the matrix output is done in jit_ob3d_draw_chunk.
			// otherwise OpenGL draws the chunk directly.
			auto result = max::jit_ob3d_draw_chunk(ob3d, chunk);
			if (result != max::JIT_ERR_NONE)
				error("failure to draw ob3d chunk");
		}
		
		//operator max::t_jit_glchunk*() const {
		//	return chunk;
		//}
		
	private:
		max::t_jit_glchunk*	chunk { nullptr };
		gl_ob*				ob3d;
	};
	
	
	
	
	
	
	
	// NOTE: For Jitter, minwrap is the nobox Jitter Object
	// we then generate another wrapper (max_jit_wrapper) around that...

	
	template<class min_class_type>
	void*
	max_jit_gl_new(max::t_symbol* s, long argc, max::t_atom* argv) {
		auto cppname = (max::t_symbol*)this_class->c_menufun;
		max_jit_wrapper* self = (max_jit_wrapper*)max::max_jit_object_alloc(this_class, cppname);
		void* o = max::jit_object_new(cppname, s);
		max_jit_mop_setup_simple(self, o, argc, argv);
		max_jit_attr_args(self, argc, argv);
		return self;
	}
	
	
	template<class min_class_type>
	void
	max_jit_gl_free(max_jit_wrapper* self) {
		max::max_jit_mop_free(self);
		max::jit_object_free(max::max_jit_obex_jitob_get(self));
		max::max_jit_object_free(self);
	}
	
	template<class min_class_type, enable_if_gl_operator<min_class_type> = 0>
	void wrap_as_max_external(const char* cppname, const char* cmaxname, void *resources, min_class_type* instance = nullptr) {
		this_class_init = true;
		
		std::string						maxname = deduce_maxclassname(cmaxname);
		std::unique_ptr<min_class_type>	dummy_instance = nullptr;

		if (!instance) {
			dummy_instance = std::make_unique<min_class_type>();
			instance = dummy_instance.get();
		}

		
		// 1. Boxless Jit Class
		
		this_jit_class = (max::t_class*)max::jit_class_new(
														   cppname,
														   (max::method)jit_new<min_class_type>,
														   (max::method)jit_free<min_class_type>,
														   sizeof( minwrap<min_class_type> ),
														   0);
		
		// add mop
		auto mop = max::jit_object_new(max::_jit_sym_jit_mop, 1, 1); // #inputs, #outputs
		max::jit_class_addadornment(this_jit_class, mop);

		// add methods
		// max::jit_class_addmethod(this_jit_class, (max::method)jit_matrix_calc<cpp_classname>, "matrix_calc", max::A_CANT, 0);

		//add attributes
		long attrflags = max::ATTR_GET_DEFER_LOW | max::ATTR_SET_USURP_LOW;
		
		for (auto& an_attribute : instance->attributes()) {
			std::string		attr_name = an_attribute.first;
			auto			attr = max::jit_object_new(
												 max::_jit_sym_jit_attr_offset,
												 attr_name.c_str(),
												 max::_jit_sym_float64,
												 attrflags,
												 (max::method)min_attr_getter<min_class_type>,
												 (max::method)min_attr_setter<min_class_type>,
												 0
												 );
			max::jit_class_addattr(this_jit_class, attr);
		}
		
		jit_class_register(this_jit_class);

		
		// 2. Max Wrapper Class
		
		this_class = max::class_new(
									maxname.c_str(),
									(max::method)max_jit_gl_new<min_class_type>,
									(max::method)max_jit_gl_free<min_class_type>,
									sizeof( max_jit_wrapper ),
									nullptr,
									max::A_GIMME,
									0);

		max::max_jit_class_obex_setup(this_class, calcoffset(max_jit_wrapper, obex));
		
		max::max_jit_class_mop_wrap(this_class, this_jit_class, 0);			// attrs & methods for name, type, dim, planecount, bang, outputmatrix, etc
		max::max_jit_class_wrap_standard(this_class, this_jit_class, 0);		// attrs & methods for getattributes, dumpout, maxjitclassaddmethods, etc
		
		max::class_addmethod(this_class, (max::method)max::max_jit_mop_assist, "assist", max::A_CANT, 0);	// standard matrix-operator (mop) assist fn
		
		// the menufun isn't used anymore, so we are repurposing it here to store the name of the jitter class we wrap
		this_class->c_menufun = (max::method)max::gensym(cppname);
		
		max::class_register(max::CLASS_BOX, this_class);
	}


}} // namespace c74::min
