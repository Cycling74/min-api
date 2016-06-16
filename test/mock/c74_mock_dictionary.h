//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
namespace max {

	using t_dictionary = t_object;

	MOCK_EXPORT t_dictionary* dictobj_findregistered_retain(t_symbol* name) {
		return nullptr;
	}

	MOCK_EXPORT t_max_err dictobj_release(t_dictionary* d) {
		return 0;
	}

	MOCK_EXPORT t_dictionary* dictionary_new(void) {
		return nullptr;
	}

}} // namespace c74::max
