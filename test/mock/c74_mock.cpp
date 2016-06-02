//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	


#include "c74_mock.h"






namespace c74 {
namespace max {


using t_dictionary = t_object;

MOCK_EXPORT t_dictionary* dictobj_findregistered_retain(t_symbol* name) {
	return nullptr;
}

MOCK_EXPORT t_max_err dictobj_release(t_dictionary* d) {
	return 0;
}


MOCK_EXPORT t_max_err object_attr_touch(t_object* x, t_symbol* attrname) {
	return 0;
}

MOCK_EXPORT t_object* attr_offset_new(const char* name, const t_symbol* type, long flags, const method mget, const method mset, long offset) {
	return nullptr;
}

MOCK_EXPORT void attr_args_process(void* x, short ac, t_atom* av) {}

MOCK_EXPORT long attr_args_offset(short ac, t_atom* av) {
	return 0;
}

MOCK_EXPORT method object_getmethod(void* x, t_symbol* s) {
	return nullptr;
}

//MOCK_EXPORT t_max_err class_register(const t_symbol* name_space, t_class* c) {
//	return 0;
//}


}}
