//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	


#include "c74_mock.h"






namespace c74 {
namespace max {


	

	MOCK_EXPORT t_max_err object_attr_touch(t_object* x, t_symbol* attrname) {
		return 0;
	}
	
	
	MOCK_EXPORT t_max_err object_attr_setvalueof(t_object* x, t_symbol* s, long argc, t_atom* argv) {
		return 0;
	}

	
	MOCK_EXPORT t_object* attr_offset_new(const char* name, const t_symbol* type, long flags, const method mget, const method mset, long offset) {
		return nullptr;
	}
	
	MOCK_EXPORT t_object* attr_offset_array_new(const char* name, t_symbol* type, long size, long flags, method mget, method mset, long offsetcount, long offset) {
		return nullptr;
	}

	

	MOCK_EXPORT void attr_args_process(void* x, short ac, t_atom* av) {}

	MOCK_EXPORT long attr_args_offset(short ac, t_atom* av) {
		return 0;
	}

	MOCK_EXPORT method object_getmethod(void* x, t_symbol* s) {
		return nullptr;
	}

	MOCK_EXPORT t_symbol* symbol_unique(void) {
		return nullptr;
	}


	MOCK_EXPORT t_max_err object_retain(t_object*) {
		return 0;
	}
	
	
	t_object* object_new_imp(void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8, void* p9, void* p10) {
		return nullptr;
	}
	
	
	using t_jit_object = t_object;
	using t_jit_err = long;
	
	MOCK_EXPORT t_jit_object* jit_object_new_imp(void* classname, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8, void* dummy) {
		return object_new_imp(classname, p1, p2, p3, p4, p5, p6, p7, p8, dummy);
	}

	
	MOCK_EXPORT t_jit_err jit_class_addadornment(void* c, t_jit_object* o) {
		return 0;
	}

	
	MOCK_EXPORT t_jit_err jit_class_addmethod(void* c, method m, const char* name, ...) {
		return 0;
	}


	MOCK_EXPORT t_jit_err jit_class_register(void* c) {
		return 0;
	}

	
	MOCK_EXPORT t_jit_err jit_class_addattr(void* c, t_jit_object* attr) {
		return 0;
	}

	
	MOCK_EXPORT t_max_err object_addattr_parse(t_object* x, const char* attrname, t_symbol* type, long flags, const char* parsestr) {
		return 0;
	}

	
	MOCK_EXPORT t_max_err class_attr_addattr_parse(t_class* c, const char* attrname, const char* attrname2, t_symbol* type, long flags, const char* parsestr) {
		return 0;
	}

	MOCK_EXPORT t_max_err class_attr_addattr_format(t_class* c, const char* attrname, const char* attrname2, const t_symbol* type, long flags, const char* fmt, ...) {
		return 0;
	}

	
	
	
	MOCK_EXPORT void *max_jit_object_alloc(t_class *mclass, t_symbol *jitter_classname) {
		return nullptr;
	}
	
	MOCK_EXPORT void max_jit_object_free(void *x) {}
	MOCK_EXPORT void max_jit_class_obex_setup(t_class *mclass, long oboffset) {}
	MOCK_EXPORT t_jit_err max_jit_class_addattr(t_class *mclass, void *attr) {
		return 0;
	}
	MOCK_EXPORT void max_jit_class_wrap_standard(t_class *mclass, t_class *jclass, long flags) {}
	MOCK_EXPORT void max_jit_class_wrap_addmethods(t_class *mclass, t_class *jclass) {}
	MOCK_EXPORT void max_jit_class_wrap_addmethods_flags(t_class *mclass, t_class *jclass, long flags) {}
	MOCK_EXPORT void max_jit_class_wrap_attrlist2methods(t_class *mclass, t_class *jclass) {}
	MOCK_EXPORT void max_jit_class_addmethod_defer(t_class *mclass, method m, char *s) {}
	MOCK_EXPORT void max_jit_class_addmethod_defer_low(t_class *mclass, method m, char *s) {}
	MOCK_EXPORT void max_jit_class_addmethod_usurp(t_class *mclass, method m, char *s) {}
	MOCK_EXPORT void max_jit_class_addmethod_usurp_low(t_class *mclass, method m, char *s) {}
	
	
	MOCK_EXPORT t_jit_err max_jit_class_mop_wrap(t_class* mclass, t_class* jclass, long flags) {
		return 0;
	}

	MOCK_EXPORT t_jit_err max_jit_mop_setup_simple(t_object*, t_object* o, long argc, t_atom* argv) {
		return 0;
	}

	MOCK_EXPORT void max_jit_mop_assist(t_object*) {} // note: prototype wrong
	MOCK_EXPORT int max_jit_mop_getoutputmode(t_object*) { return 0; }
	MOCK_EXPORT void max_jit_mop_outputmatrix(t_object*) {}
	MOCK_EXPORT void max_jit_mop_free(t_object*) {}
	MOCK_EXPORT void* max_jit_obex_jitob_get(t_object*) { return nullptr; }
	MOCK_EXPORT void* max_jit_obex_adornment_get(t_object* self, t_symbol* name) { return nullptr; }
	MOCK_EXPORT void max_jit_attr_args(t_object*, long argc, t_atom* argv) {}
	MOCK_EXPORT void* jit_object_alloc(t_class* c) { return object_alloc(c); }
	MOCK_EXPORT void jit_object_free(t_object* x) { object_free(x); }
	


#ifdef __APPLE__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wunused-variable"
#endif
	static const int JIT_MATRIX_MAX_DIMCOUNT = 32; 			///< maximum dimension count @ingroup jitter
	static const int JIT_MATRIX_MAX_PLANECOUNT = 32; 			///< maximum plane count @ingroup jitter
#ifdef __APPLE__
	#pragma clang diagnostic pop
#endif

	struct t_jit_matrix_info {
		long		size = 0;			///< in bytes (0xFFFFFFFF=UNKNOWN)
		t_symbol*	type = nullptr;			///< primitifve type (char, long, float32, or float64)
		long		flags = 0;			///< flags to specify data reference, handle, or tightly packed
		long		dimcount = 0;		///< number of dimensions
		long		dim[JIT_MATRIX_MAX_DIMCOUNT];		///< dimension sizes
		long		dimstride[JIT_MATRIX_MAX_DIMCOUNT]; ///< stride across dimensions in bytes
		long		planecount = 0;		///< number of planes
	};

	MOCK_EXPORT void jit_parallel_ndim_simplecalc1(method fn, void *data, long dimcount, long *dim, long planecount, t_jit_matrix_info *minfo1, char *bp1, long flags1)
	{}
	MOCK_EXPORT void jit_parallel_ndim_simplecalc2(method fn, void *data, long dimcount, long *dim, long planecount, t_jit_matrix_info *minfo1, char *bp1,
		t_jit_matrix_info *minfo2, char *bp2, long flags1, long flags2)
	{}

	#ifndef C74_X64
	MOCK_EXPORT void* jit_object_new(t_symbol* name) { return jit_object_new_imp(name, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr); }
	#endif

}}
