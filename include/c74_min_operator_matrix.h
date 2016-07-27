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
		: in_info	{ a_in_info }
		, bip		{ ip }
		, out_info	{ a_out_info }
		, bop		{ op }
		{}

		
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

	/// @param s	The name of the object is passed as an argument to support object-mappings.
	///		In such cases we might not know what the object name is at compile time.
	template<class min_class_type>
	max::t_object* jit_new(max::t_symbol* s){
		minwrap<min_class_type>*	self = (minwrap<min_class_type>*)max::jit_object_alloc(this_jit_class);
        
        self->min_object.assign_instance((max::t_object*)self);
		min_ctor(self, {});
		self->min_object.set_classname(s);
		self->min_object.try_call("setup");
		
		return (max::t_object*)self;
	}
	
	template<class min_class_type>
	void jit_free(minwrap<min_class_type>* self){
		self->cleanup();
		self->min_object.~min_class_type(); // placement delete
	}
	
	
	
	struct max_jit_wrapper {
		max::t_object	ob;
		void*			obex;
	};
	
	
	template<class min_class_type>
	void* max_jit_mop_new(max::t_symbol* s, long argc, max::t_atom* argv) {
		atom_reference		args(argc, argv);
        long				attrstart	= attr_args_offset((short)args.size(), args.begin());
		auto				cppname		= (max::t_symbol*)this_class->c_menufun;
		max_jit_wrapper*	self		= (max_jit_wrapper*)max::max_jit_object_alloc(this_class, cppname);
		void*				o			= max::jit_object_new(cppname, s);
		
		max_jit_mop_setup_simple(self, o, args.size(), args.begin());
		max_jit_attr_args(self, (short)args.size(), args.begin());
        
        auto	job = (minwrap<min_class_type>*)o;
        job->min_object.try_call("maxob_setup", atoms(args.begin(), args.begin()+attrstart));
        
		return self;
	}
	
	
	template<class min_class_type>
	void max_jit_mop_free(max_jit_wrapper* self) {
		max::max_jit_mop_free(self);
		max::jit_object_free(max::max_jit_obex_jitob_get(self));
		max::max_jit_object_free(self);
	}
 
	
	// We are using a C++ template to process a vector of the matrix for any of the given types.
	// Thus, we don't need to duplicate the code for each datatype.
	template<class min_class_type, typename U, enable_if_matrix_operator<min_class_type> = 0>
	void jit_calculate_vector(minwrap<min_class_type>* self, const matrix_info& info, long n, long i, max::t_jit_op_info* in, max::t_jit_op_info* out) {
		auto ip = (in ? ((U*)in->p) : nullptr);
		auto op = ((U*)out->p);
		auto is = (in ? in->stride : 0);
		auto os = out->stride;
		const auto step = os / info.planecount();
		const bool planematch = (info.in_info->planecount == info.out_info->planecount);
        
		if (planematch && info.planecount() == 1) {
			for (auto j=0; j<n; ++j) {
				matrix_coord position(j, i);
                U val = (ip ? *(ip) : 0);
                const std::array<U,1> tmp = {{ val }};
                const std::array<U,1> out = self->min_object.calc_cell(tmp, info, position);
                
				*(op) = out[0];
				if(ip) ip += is;
				op += os;
			}
		}
		else if (planematch && info.planecount() == 4) {
			for (auto j=0; j<n; ++j) {
				matrix_coord position(j, i);
                U v1=(ip?*(ip):0), v2=(ip?*(ip+step):0), v3=(ip?*(ip+step*2):0), v4=(ip?*(ip+step*3):0);
                const std::array<U,4> tmp = {{ v1,v2,v3,v4 }};
                const std::array<U,4> out = self->min_object.calc_cell(tmp, info, position);
                
				*(op) = out[0];
				*(op+step) = out[1];
				*(op+step*2) = out[2];
				*(op+step*3) = out[3];
				
				if(ip) ip += is;
				op += os;
			}
		}
		else {
			for (auto j=0; j<n; ++j) {
                const auto instep = is / info.in_info->planecount;
                const auto outstep = os / info.out_info->planecount;
				std::array<U,max::JIT_MATRIX_MAX_PLANECOUNT> tmp;// = { *(ip), *(ip+step), *(ip+step*2), *(ip+step*3) };
				
                if(ip) {
                    for (auto k=0; k<info.in_info->planecount; ++k)
                        tmp[k] = *(ip+instep*k);
                }
				
				matrix_coord position(j, i);
				const std::array<U,max::JIT_MATRIX_MAX_PLANECOUNT> out = self->min_object.calc_cell(tmp, info, position);
				
				for (auto k=0; k<info.out_info->planecount; ++k)
					*(op+outstep*k) = out[k];
				
				if(ip) ip += is;
				op += os;
			}
		}
	}
	

	// We also use a C+ template for the loop that wraps the call to jit_simple_vector(),
	// further reducing code duplication in jit_simple_calculate_ndim().
	// The calls into these templates should be inlined by the compiler, eliminating concern about any added function call overhead.
	template<class min_class_type, typename U>
	typename enable_if<is_base_of<matrix_operator, min_class_type>::value>::type
	jit_calculate_ndim_loop(minwrap<min_class_type>* self, long n, max::t_jit_op_info* in_opinfo, max::t_jit_op_info* out_opinfo, max::t_jit_matrix_info* in_minfo, max::t_jit_matrix_info* out_minfo, char* bip, char* bop, long* dim, long planecount, long datasize) {
		matrix_info info((in_minfo ? in_minfo : out_minfo), (bip ? bip : bop), out_minfo, bop);
		for (auto i=0; i<dim[1]; i++) {
			if(in_opinfo) in_opinfo->p  = bip + i * in_minfo->dimstride[1];
			out_opinfo->p = bop + i * out_minfo->dimstride[1];
			jit_calculate_vector<min_class_type,U>(self, info, n, i, in_opinfo, out_opinfo);
		}
	}

	
	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	void jit_calculate_ndim(minwrap<min_class_type>* self, long dimcount, long* dim, long planecount, max::t_jit_matrix_info *in_minfo, char* bip, max::t_jit_matrix_info* out_minfo, char* bop) {
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
						jit_calculate_ndim_loop<min_class_type, uchar>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 1);
					else if (in_minfo->type == max::_jit_sym_long)
						jit_calculate_ndim_loop<min_class_type, int>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float32)
						jit_calculate_ndim_loop<min_class_type, float>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 4);
					else if (in_minfo->type == max::_jit_sym_float64)
						jit_calculate_ndim_loop<min_class_type, double>(self, n, &in_opinfo, &out_opinfo, in_minfo, out_minfo, bip, bop, dim, planecount, 8);
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
	
	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	void jit_calculate_ndim_single(minwrap<min_class_type>* self, long dimcount, long* dim, long planecount, max::t_jit_matrix_info* out_minfo, char* bop) {
		if (dimcount < 1)
			return; // safety
        
		max::t_jit_op_info	out_opinfo;
		
		switch (dimcount) {
			case 1:
				dim[1] = 1;
				// (fall-through to next case is intentional)
			case 2:
				{
					// if planecount is the same then flatten planes - treat as single plane data for speed
					auto n = dim[0];
					out_opinfo.stride = out_minfo->dim[0]>1 ? out_minfo->planecount : 0;
					
					if (out_minfo->type == max::_jit_sym_char)
						jit_calculate_ndim_loop<min_class_type, uchar>(self, n, NULL, &out_opinfo, NULL, out_minfo, NULL, bop, dim, planecount, 1);
					else if (out_minfo->type == max::_jit_sym_long)
						jit_calculate_ndim_loop<min_class_type, int>(self, n, NULL, &out_opinfo, NULL, out_minfo, NULL, bop, dim, planecount, 1);
					else if (out_minfo->type == max::_jit_sym_float32)
						jit_calculate_ndim_loop<min_class_type, float>(self, n, NULL, &out_opinfo, NULL, out_minfo, NULL, bop, dim, planecount, 1);
					else if (out_minfo->type == max::_jit_sym_float64)
						jit_calculate_ndim_loop<min_class_type, double>(self, n, NULL, &out_opinfo, NULL, out_minfo, NULL, bop, dim, planecount, 1);
				}
				break;
			default:
				for	(auto i=0; i<dim[dimcount-1]; i++) {
					auto op = bop + i * out_minfo->dimstride[dimcount-1];
					jit_calculate_ndim_single(self, dimcount-1, dim, planecount, out_minfo, op);
				}
		}
	}
    
	
	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	void jit_matrix_docalc(minwrap<min_class_type>* self, max::t_object* inputs, max::t_object* outputs) {
		max::t_jit_err			err = max::JIT_ERR_NONE;
		auto					in_mop_io = (max::t_object*)max::object_method(inputs, max::_jit_sym_getindex, 0);
		auto					out_mop_io = (max::t_object*)max::object_method(outputs, max::_jit_sym_getindex, 0);
		auto					in_matrix 	= (max::t_object*)max::object_method(in_mop_io, k_sym_getmatrix);
		auto					out_matrix 	= (max::t_object*)max::object_method(out_mop_io, k_sym_getmatrix);
		
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
												   (max::method)jit_calculate_ndim<min_class_type>,
												   self,
												   dimcount, dim, planecount, &in_minfo, in_bp, &out_minfo, out_bp,
												   0, 0
				);
			}

			max::object_method(out_matrix, max::_jit_sym_lock, out_savelock);
			max::object_method(in_matrix, max::_jit_sym_lock, in_savelock);
		}
		throw err;
	}

	
	template<class min_class_type>
	max::t_jit_err jit_matrix_calc(minwrap<min_class_type>* self, max::t_object* inputs, max::t_object* outputs) {
		try {
			jit_matrix_docalc(self, inputs, outputs);
			return 0;
		}
		catch (max::t_jit_err& err) {
			return err;
		}
	}
	
	
	template<class min_class_type, enable_if_matrix_operator<min_class_type> = 0>
	void min_jit_mop_outputmatrix(max_jit_wrapper* self) {
        auto jitob = (minwrap<min_class_type>*)max::max_jit_obex_jitob_get(self);
        long outputmode = max::max_jit_mop_getoutputmode(self);
        void *mop = max::max_jit_obex_adornment_get(self, max::_jit_sym_jit_mop);
 
        if (outputmode && mop && outputmode==1) { //always output unless output mode is none
            max::t_object*	outputs = (max::t_object*)max::object_method((max::t_object*)mop,max::_jit_sym_getoutputlist);
            max::t_jit_err	err = max::JIT_ERR_NONE;
            auto			out_mop_io = (max::t_object*)max::object_method(outputs, max::_jit_sym_getindex, 0);
            auto			out_matrix = (max::t_object*)max::object_method(out_mop_io, k_sym_getmatrix);
            
            if (!self || !out_matrix){
                err = max::JIT_ERR_INVALID_PTR;
            }
            else {
                auto					out_savelock = max::object_method(out_matrix, max::_jit_sym_lock, (void*)1);
                max::t_jit_matrix_info	out_minfo;
				char*					out_bp = nullptr;

                max::object_method(out_matrix, max::_jit_sym_getinfo, &out_minfo);
                max::object_method(out_matrix, max::_jit_sym_getdata, &out_bp);
                
                if (!out_bp)
                    err = max::JIT_ERR_INVALID_OUTPUT;
                else {
                    max::jit_parallel_ndim_simplecalc1(
                                                       (max::method)jit_calculate_ndim_single<min_class_type>, jitob,
                                                       out_minfo.dimcount, out_minfo.dim, out_minfo.planecount, &out_minfo, out_bp, 0
                    );
                }

                max::object_method(out_matrix, max::_jit_sym_lock, out_savelock);
            }
            max::max_jit_mop_outputmatrix(self);
        }
        else {
            max::max_jit_mop_outputmatrix(self);
        }
    }
	
	

}} // namespace c74::min
