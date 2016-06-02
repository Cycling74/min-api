/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_jitter.h"

namespace c74 {
namespace min {

	class matrix_operator_base {};

	class matrix_operator : public matrix_operator_base {};
	
	
	using pixel = std::array<uchar,4>;

	template<class matrix_type, size_t planecount>
	using cell = std::array<matrix_type, planecount>;

	enum {
		alpha = 0,
		red,
		green,
		blue
	};
	
	class matrix_coord {
	public:
		matrix_coord(long x, long y) {
			position[0] = x;
			position[1] = y;
		}
		
		long x() const {
			return position[0];
		}
		
		long y() const {
			return position[1];
		}
		
		long position[max::JIT_MATRIX_MAX_DIMCOUNT];
	};
	
	class matrix_info {
	public:
		matrix_info(max::t_jit_matrix_info* a_in_info, char* ip, max::t_jit_matrix_info* a_out_info, char* op)
		: in_info(a_in_info)
		, bip(ip)
		, out_info(a_out_info)
		, bop(op)
		{
		}

		
		long planecount() const {
			return in_info->planecount;
		}

		long dimcount() const {
			return in_info->dimcount;
		}

		long width() const {
			return in_info->dim[0];
		}

		long height() const {
			return in_info->dim[1];
		}

		
		
		template<class matrix_type, size_t planecount>
		const std::array<matrix_type,planecount> in_cell(const matrix_coord& coord) const {
			auto p = (char*)bip;
			
			for (auto j=0; j < in_info->dimcount; ++j)
				p += coord.position[j] * in_info->dimstride[j];
			
			std::array<matrix_type,planecount> pa;
			
			auto p2 = (matrix_type*)p;
			for (auto plane=0; plane<planecount; ++plane)
				pa[plane] = *(p2+plane);
			return pa;
		}
		
		template<class matrix_type, size_t planecount>
		const std::array<matrix_type,planecount> in_cell(int x, int y) const {
			matrix_coord coord(x, y);
			return in_cell<matrix_type,planecount>(coord);
		}


	
		const pixel in_pixel(const matrix_coord& coord) const {
			uchar* p = (uchar*)bip;
			
			for (auto j=0; j < in_info->dimcount; ++j)
				p += coord.position[j] * in_info->dimstride[j];

			const pixel pa = {{ *(p), *(p+1), *(p+2), *(p+3) }};
			return pa;
		}
		
		
		const pixel in_pixel(int x, int y) const {
			matrix_coord coord(x, y);
			return in_pixel(coord);
		}
		

		pixel out_pixel(const matrix_coord&) {
			// TODO: implement
			pixel a;
			return a;
		}

		
		max::t_jit_matrix_info*	in_info;
		char*		bip;
		max::t_jit_matrix_info*	out_info;
		char*		bop;
		
	};
	
	
	
	// this is for the jitter object (the normal one is used for the max wrapper of that)
	static max::t_class* this_jit_class = nullptr;

	
	// NOTE: For Jitter, minwrap is the nobox Jitter Object
	// we then generate another wrapper (max_jit_wrapper) around that...

	
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
	
	
	template<class cpp_classname>
	void*
	max_jit_mop_new(max::t_symbol* s, long argc, max::t_atom* argv) {
		auto cppname = (max::t_symbol*)c74::min::this_class->c_menufun;
		max_jit_wrapper* self = (max_jit_wrapper*)max::max_jit_object_alloc(this_class, cppname);
		void* o = max::jit_object_new(cppname);
		max_jit_mop_setup_simple(self, o, argc, argv);
		max_jit_attr_args(self, argc, argv);
		return self;
	}
	
	
	template<class cpp_classname>
	void
	max_jit_mop_free(max_jit_wrapper* self) {
		max::max_jit_mop_free(self);
		max::jit_object_free(max::max_jit_obex_jitob_get(self));
		max::max_jit_object_free(self);
	}
 
	
	// We are using a C++ template to process a vector of the matrix for any of the given types.
	// Thus, we don't need to duplicate the code for each datatype.
	template<class cpp_classname, typename U>
	typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type
	jit_calculate_vector(minwrap<cpp_classname>* self, const matrix_info& info, long n, long i, max::t_jit_op_info* in, max::t_jit_op_info* out) {
		auto ip = ((U*)in->p);
		auto op = ((U*)out->p);
		auto is = in->stride;
		auto os = out->stride;
		const auto step = is / info.planecount();
		
		if (info.planecount() == 1) {
			for (auto j=0; j<n; ++j) {
				matrix_coord position(j, i);
				const std::array<U,1> tmp = {{ *(ip) }};
				const std::array<U,1> out = self->obj.calc_cell(tmp, info, position);
				*(op) = out[0];
				ip += is;
				op += os;
			}
		}
		else if (info.planecount() == 4) {
			for (auto j=0; j<n; ++j) {
				matrix_coord position(j, i);
				const std::array<U,4> tmp = {{ *(ip), *(ip+step), *(ip+step*2), *(ip+step*3) }};
				const std::array<U,4> out = self->obj.calc_cell(tmp, info, position);
				*(op) = out[0];
				*(op+step) = out[1];
				*(op+step*2) = out[2];
				*(op+step*3) = out[3];
				
				ip += is;
				op += os;
			}
		}
		else {
			for (auto j=0; j<n; ++j) {
				std::array<U,c74::max::JIT_MATRIX_MAX_PLANECOUNT> tmp;// = { *(ip), *(ip+step), *(ip+step*2), *(ip+step*3) };
				
				for (auto k=0; k<info.planecount(); ++k)
					tmp[k] = *(ip+step*k);
				
				matrix_coord position(j, i);
				const std::array<U,c74::max::JIT_MATRIX_MAX_PLANECOUNT> out = self->obj.calc_cell(tmp, info, position);
				
				for (auto k=0; k<info.planecount(); ++k)
					*(op+step*k) = out[k];
				
				ip += is;
				op += os;
			}
		}
	}
	

	// We also use a C+ template for the loop that wraps the call to jit_simple_vector(),
	// further reducing code duplication in jit_simple_calculate_ndim().
	// The calls into these templates should be inlined by the compiler, eliminating concern about any added function call overhead.
	template<class cpp_classname, typename U>
	typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type
	jit_calculate_ndim_loop(minwrap<cpp_classname>* self, long n, max::t_jit_op_info* in_opinfo, max::t_jit_op_info* out_opinfo, max::t_jit_matrix_info* in_minfo, max::t_jit_matrix_info* out_minfo, char* bip, char* bop, long* dim, long planecount, long datasize) {
		matrix_info info(in_minfo, bip, out_minfo, bop);
		for (auto i=0; i<dim[1]; i++) {
			in_opinfo->p  = bip + i * in_minfo->dimstride[1];
			out_opinfo->p = bop + i * out_minfo->dimstride[1];
			jit_calculate_vector<cpp_classname,U>(self, info, n, i, in_opinfo, out_opinfo);
		}
	}

	template<class cpp_classname>
	using enable_if_matrix_operator = typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type;
	
	template<class cpp_classname>
//	typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type
	enable_if_matrix_operator<cpp_classname>
	jit_calculate_ndim(minwrap<cpp_classname>* self, long dimcount, long* dim, long planecount, max::t_jit_matrix_info *in_minfo, char* bip, max::t_jit_matrix_info* out_minfo, char* bop) {
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
					in_opinfo.stride =  in_minfo->dim[0]>1  ? in_minfo->planecount  : 0;
					out_opinfo.stride = out_minfo->dim[0]>1 ? out_minfo->planecount : 0;
					
					if (in_minfo->type == max::_jit_sym_char)
						jit_calculate_ndim_loop<cpp_classname, uchar>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 1);
					else if (in_minfo->type == max::_jit_sym_long)
						jit_calculate_ndim_loop<cpp_classname, int>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float32)
						jit_calculate_ndim_loop<cpp_classname, float>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float64)
						jit_calculate_ndim_loop<cpp_classname, double>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 8);
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
	
	
//	template <typename Type, typename std::enable_if<condition<Type>::value, int>::type = 0>
//	return_type foo(const Type&);

//	template <typename cpp_classname, enable_if_matrix_operator<cpp_classname> = 0>
//	return_type foo(const Type&);
//
	template<class cpp_classname>
//	typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type
	enable_if_matrix_operator<cpp_classname>
	/*max::t_jit_err*/ jit_matrix_calc(minwrap<cpp_classname>* self, max::t_object* inputs, max::t_object* outputs) {
		max::t_jit_err			err = max::JIT_ERR_NONE;
		auto					in_mop_io = (max::t_object*)max::object_method(inputs, max::_jit_sym_getindex, 0);
		auto					out_mop_io = (max::t_object*)max::object_method(outputs, max::_jit_sym_getindex, 0);
		auto					in_matrix 	= (max::t_object*)max::object_method(in_mop_io, max::gensym("getmatrix"));
		auto					out_matrix 	= (max::t_object*)max::object_method(out_mop_io, max::gensym("getmatrix"));
		
		if (!self || !in_matrix || !out_matrix){
			err = max::JIT_ERR_INVALID_PTR;
		}
		else {
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
					if ((in_minfo.dim[i]<dim[i]) && in_minfo.dim[i]>1) {
						dim[i] = in_minfo.dim[i];
					}
				}
				
				max::jit_parallel_ndim_simplecalc2(
												   (c74::max::method)jit_calculate_ndim<cpp_classname>,
												   self,
												   dimcount, dim, planecount, &in_minfo, in_bp, &out_minfo, out_bp,
												   0, 0
				);
			}

			max::object_method(out_matrix, max::_jit_sym_lock, out_savelock);
			max::object_method(in_matrix, max::_jit_sym_lock, in_savelock);
		}
		// TODO: can't return an error
		// return err;
	}


}} // namespace c74::min







template<class cpp_classname>
typename std::enable_if<std::is_base_of<c74::min::matrix_operator, cpp_classname>::value>::type
define_min_external(const char* cppname, const char* cmaxname, void *resources) {
	std::string		maxname = c74::min::deduce_maxclassname(cmaxname);

	
	
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
											   maxname.c_str(),
											   (c74::max::method)c74::min::max_jit_mop_new<cpp_classname>,
											   (c74::max::method)c74::min::max_jit_mop_free<cpp_classname>,
											   sizeof( c74::min::max_jit_wrapper ),
											   nullptr,
											   c74::max::A_GIMME,
											   0
											   );

	c74::max::max_jit_class_obex_setup(c74::min::this_class, calcoffset(c74::min::max_jit_wrapper, obex));
	
	c74::max::max_jit_class_mop_wrap(c74::min::this_class, c74::min::this_jit_class, 0);			// attrs & methods for name, type, dim, planecount, bang, outputmatrix, etc
	c74::max::max_jit_class_wrap_standard(c74::min::this_class, c74::min::this_jit_class, 0);		// attrs & methods for getattributes, dumpout, maxjitclassaddmethods, etc
	
	c74::max::class_addmethod(c74::min::this_class, (c74::max::method)c74::max::max_jit_mop_assist, "assist", c74::max::A_CANT, 0);	// standard matrix-operator (mop) assist fn
	
	// the menufun isn't used anymore, so we are repurposing it here to store the name of the jitter class we wrap
	c74::min::this_class->c_menufun = (c74::max::method)c74::max::gensym(cppname);
	
	c74::max::class_register(c74::max::CLASS_BOX, c74::min::this_class);
}
