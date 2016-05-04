#pragma once

#include "c74_jitter.h"

namespace c74 {
namespace min {

	
	// this is for the jitter object (the normal one is used for the max wrapper of that)
	static max::t_class* this_jit_class = nullptr;

	
	// NOTE: For Jitter, the minwrap is the Jitter Object -- we then generate another wrapper around that...
	
	template<class T>
	struct minwrap < T, typename std::enable_if< std::is_base_of< min::matrix_object, T>::value >::type > {
		max::t_object 		header;
		T					obj;
		
		void setup() {
			//max::dsp_setup((max::t_pxobject*)self, self->obj.inlets.size());
		}
		
		void cleanup() {}
	};
	
	
	template<class T>
	max::t_object* jit_new(){
		minwrap<T>*	self = (minwrap<T>*)max::jit_object_alloc(this_jit_class);
		
		atoms args;
		new(&self->obj) T(args); // placement new
		self->obj.maxobj = (max::t_object*)self;
		
		return (max::t_object*)self;
	}
	
	template<class T>
	void jit_free(minwrap<T>* self){
		self->cleanup();
		self->obj.~T(); // placement delete
	}
	
	
	
	struct max_jit_wrapper {
		max::t_object	ob;
		void*			obex;
	};
	
	
	template<class T>
	void* max_jit_new(max::t_symbol* s, long argc, max::t_atom* argv) {
		max_jit_wrapper* self = (max_jit_wrapper*)max::max_jit_object_alloc(this_class, c74::max::gensym("jit_clamp"));
// TODO: hard-wired jit_clip above
		void* o = max::jit_object_new(max::gensym("jit_clamp"));
// TODO: hard-wired jit_clip above
		max_jit_mop_setup_simple(self, o, argc, argv);
		max_jit_attr_args(self, argc, argv);

		return self;
	}
	
	
	void max_jit_free(max_jit_wrapper* self) {
		max::max_jit_mop_free(self);
		max::jit_object_free(max::max_jit_obex_jitob_get(self));
		max::max_jit_object_free(self);
	}
 
	
	// We are using a C++ template to process a vector of the matrix for any of the given types.
	// Thus, we don't need to duplicate the code for each datatype.
	template<class T, typename U>
	void jit_calculate_vector(minwrap<T>* self, long n, max::t_jit_op_info* in, max::t_jit_op_info* out) {
		auto ip = ((U*)in->p);
		auto op = ((U*)out->p);
		auto is = in->stride;
		auto os = out->stride;
		
		if ((is==1) && (os==1)) {
			++n;
			--op;
			--ip;
			while (--n) {
				const auto tmp = *++ip;
				*++op = self->obj.calc_cell(tmp);
			}
		}
		else {
			while (n--) {
				const auto tmp = *ip;
				*op = self->obj.calc_cell(tmp);
				ip += is;
				op += os;
			}
		}
	}
	

	// We also use a C+ template for the loop that wraps the call to jit_simple_vector(),
	// further reducing code duplication in jit_simple_calculate_ndim().
	// The calls into these templates should be inlined by the compiler, eliminating concern about any added function call overhead.
	template<class T, typename U>
	void jit_calculate_ndim_loop(minwrap<T>* self, long n, max::t_jit_op_info* in_opinfo, max::t_jit_op_info* out_opinfo, max::t_jit_matrix_info* in_minfo, max::t_jit_matrix_info* out_minfo, char* bip, char* bop, long* dim, long planecount, long datasize) {
		for (auto i=0; i<dim[1]; i++) {
			for (auto j=0; j<planecount; j++) {
				in_opinfo->p  = bip + i * in_minfo->dimstride[1]  + (j % in_minfo->planecount) * datasize;
				out_opinfo->p = bop + i * out_minfo->dimstride[1] + (j % out_minfo->planecount) * datasize;
				jit_calculate_vector<T,U>(self, n, in_opinfo, out_opinfo);
			}
		}
	}

	
	template<class T>
	void jit_calculate_ndim(minwrap<T>* self, long dimcount, long* dim, long planecount, max::t_jit_matrix_info *in_minfo, char* bip, max::t_jit_matrix_info* out_minfo, char* bop) {
		if (dimcount < 1)
			return; // safety

		max::t_jit_op_info	in_opinfo;
		max::t_jit_op_info	out_opinfo;
		
		switch (dimcount) {
			case 1:
				dim[1] = 1;
				// (fall-through to next case is intentional)
			case 2:
				{
					// if planecount is the same then flatten planes - treat as single plane data for speed
					auto n = dim[0];
					if ((in_minfo->dim[0] > 1) && (out_minfo->dim[0] > 1) && (in_minfo->planecount == out_minfo->planecount)) {
						in_opinfo.stride = 1;
						out_opinfo.stride = 1;
						n *= planecount;
						planecount = 1;
					}
					else {
						in_opinfo.stride =  in_minfo->dim[0]>1  ? in_minfo->planecount  : 0;
						out_opinfo.stride = out_minfo->dim[0]>1 ? out_minfo->planecount : 0;
					}
					
					if (in_minfo->type == max::_jit_sym_char)
						jit_calculate_ndim_loop<T, uchar>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 1);
					else if (in_minfo->type == max::_jit_sym_long)
						jit_calculate_ndim_loop<T, int>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float32)
						jit_calculate_ndim_loop<T, float>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float64)
						jit_calculate_ndim_loop<T, double>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 8);
				}
				break;
			default:
				for	(auto i=0; i<dim[dimcount-1]; i++) {
					auto ip = bip + i * in_minfo->dimstride[dimcount-1];
					auto op = bop + i * out_minfo->dimstride[dimcount-1];
					jit_calculate_ndim(self, dimcount-1, dim, planecount, in_minfo, ip, out_minfo, op);
				}
		}
	}
	
	
	template<class T>
	max::t_jit_err jit_matrix_calc(minwrap<T>* self, max::t_object* inputs, max::t_object* outputs) {
		max::t_jit_err			err = max::JIT_ERR_NONE;
		auto					in_mop_io = (max::t_object*)max::object_method(inputs, max::_jit_sym_getindex, 0);
		auto					out_mop_io = (max::t_object*)max::object_method(outputs, max::_jit_sym_getindex, 0);
		auto					in_matrix 	= (max::t_object*)max::object_method(in_mop_io, max::gensym("getmatrix"));
		auto					out_matrix 	= (max::t_object*)max::object_method(out_mop_io, max::gensym("getmatrix"));
		
		if (!self || !in_matrix || !out_matrix)
			return max::JIT_ERR_INVALID_PTR;

		auto in_savelock = max::object_method(in_matrix, max::_jit_sym_lock, (void*)1);
		auto out_savelock = max::object_method(out_matrix, max::_jit_sym_lock, (void*)1);
		
		max::t_jit_matrix_info in_minfo;
		max::t_jit_matrix_info out_minfo;
		max::object_method(in_matrix, max::_jit_sym_getinfo, &in_minfo);
		max::object_method(out_matrix, max::_jit_sym_getinfo, &out_minfo);
		
		char* in_bp = nullptr;
		char* out_bp = nullptr;
		max::object_method(in_matrix, max::_jit_sym_getdata, &in_bp);
		max::object_method(out_matrix, max::_jit_sym_getdata, &out_bp);
		
		if (!in_bp)
			err = max::JIT_ERR_INVALID_INPUT;
		else if (!out_bp)
			err = max::JIT_ERR_INVALID_OUTPUT;
		else if (in_minfo.type != out_minfo.type)
			err = max::JIT_ERR_MISMATCH_TYPE;
		
		if (in_minfo.type == out_minfo.type && in_bp && out_bp) {
			long dim[max::JIT_MATRIX_MAX_DIMCOUNT];
			auto dimcount   = out_minfo.dimcount;
			auto planecount = out_minfo.planecount;
			
			for (auto i=0; i<dimcount; ++i) {
				//if dimsize is 1, treat as infinite domain across that dimension.
				//otherwise truncate if less than the output dimsize
				dim[i] = out_minfo.dim[i];
				if ((in_minfo.dim[i]<dim[i])&&in_minfo.dim[i]>1) {
					dim[i] = in_minfo.dim[i];
				}
			}
			
			max::jit_parallel_ndim_simplecalc2(
											   (c74::max::method)jit_calculate_ndim<T>,
											   self,
											   dimcount, dim, planecount, &in_minfo, in_bp, &out_minfo, out_bp,
											   0, 0
			);
		}

		max::object_method(out_matrix, max::_jit_sym_lock, out_savelock);
		max::object_method(in_matrix, max::_jit_sym_lock, in_savelock);
		return err;
	}


}} // namespace c74::min







template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::matrix_object, cpp_classname>::value>::type
define_min_external(const char* cppname, const char* maxname, void *resources)
{
	// 1. Boxless Jit Class

	c74::min::atoms	a;
	cpp_classname	dummy(a);
	
	c74::min::this_jit_class = (c74::max::t_class*)c74::max::jit_class_new(
																		   cppname,
																		   (c74::max::method)c74::min::jit_new<cpp_classname>,
																		   (c74::max::method)c74::min::jit_free<cpp_classname>,
																		   sizeof( c74::min::minwrap<cpp_classname> ),
																		   0);
	
	//add mop
	auto mop = c74::max::jit_object_new(c74::max::_jit_sym_jit_mop, 1, 1); // #inputs, #outputs
	c74::max::jit_class_addadornment(c74::min::this_jit_class, mop);

	//add methods
	c74::max::jit_class_addmethod(c74::min::this_jit_class, (c74::max::method)c74::min::jit_matrix_calc<cpp_classname>, "matrix_calc", c74::max::A_CANT, 0);

	//add attributes
	long attrflags = c74::max::ATTR_GET_DEFER_LOW | c74::max::ATTR_SET_USURP_LOW;
	

	for (auto& an_attribute : dummy.attributes) {
		std::string		attr_name = an_attribute.first;
		auto			attr = c74::max::jit_object_new(
											 c74::max::_jit_sym_jit_attr_offset,
											 attr_name.c_str(),
											 c74::max::_jit_sym_float64,
											 attrflags,
											 (c74::max::method)c74::min::min_attr_getter<cpp_classname>,
											 (c74::max::method)c74::min::min_attr_setter<cpp_classname>,
											 0
											 );
		c74::max::jit_class_addattr(c74::min::this_jit_class, attr);
	}
	
	jit_class_register(c74::min::this_jit_class);

	
	// 2. Max Wrapper Class
	
	c74::min::this_class = c74::max::class_new(
											   maxname,
											   (c74::max::method)c74::min::max_jit_new<cpp_classname>,
											   (c74::max::method)c74::min::max_jit_free,
											   sizeof( c74::min::max_jit_wrapper ),
											   nullptr,
											   c74::max::A_GIMME,
											   0
											   );

	c74::max::max_jit_class_obex_setup(c74::min::this_class, calcoffset(c74::min::max_jit_wrapper, obex));
	
	c74::max::max_jit_class_mop_wrap(c74::min::this_class, c74::min::this_jit_class, 0);			// attrs & methods for name, type, dim, planecount, bang, outputmatrix, etc
	c74::max::max_jit_class_wrap_standard(c74::min::this_class, c74::min::this_jit_class, 0);		// attrs & methods for getattributes, dumpout, maxjitclassaddmethods, etc
	
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::max::max_jit_mop_assist, "assist", c74::max::A_CANT, 0);	// standard matrix-operator (mop) assist fn
	
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}

