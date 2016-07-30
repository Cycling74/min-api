/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_max.h"
#include "c74_msp.h"
#include <array>
#include <atomic>
#include <chrono>
#include <deque>
#include <iostream>
#include <list>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>


namespace c74 {
namespace min {

	// types
	
	using uchar = unsigned char;

	using sample = double;

	template<size_t count>
	using samples = std::array<sample, count>;

	using sample_vector = std::vector<sample>;
	
	
	// Very selective group from the STL used only for making common
	// template SFINAE code more readable

	using std::enable_if;
	using std::is_base_of;
	using std::is_same;
	using std::is_enum;
	
	
	// Helper code for type/template selection

	class symbol;
	class matrix_operator_base;
	class gl_operator_base;
	class sample_operator_base;
	class perform_operator_base;
	
	template<class T>
	using is_class = std::is_class<T>;
	
	template<class T>
	using is_symbol = is_same<T, symbol>;

	template<class min_class_type>
	using enable_if_matrix_operator = typename enable_if<is_base_of<matrix_operator_base, min_class_type>::value, int>::type;
	
	template<class min_class_type>
	using enable_if_not_matrix_operator = typename enable_if<!is_base_of<matrix_operator_base, min_class_type>::value, int>::type;

	template<class min_class_type>
	using enable_if_gl_operator = typename enable_if<is_base_of<gl_operator_base, min_class_type>::value, int>::type;

	template<class min_class_type>
	using enable_if_sample_operator = typename enable_if<is_base_of<sample_operator_base, min_class_type>::value, int>::type;

	template<class min_class_type>
	using enable_if_perform_operator = typename enable_if<is_base_of<perform_operator_base, min_class_type>::value, int>::type;

	template<class min_class_type>
	using type_enable_if_audio_class = typename enable_if<is_base_of<perform_operator_base, min_class_type>::value || is_base_of<sample_operator_base, min_class_type>::value >::type;

	template<class min_class_type>
	using type_enable_if_not_audio_class = typename enable_if<!is_base_of<perform_operator_base, min_class_type>::value && !is_base_of<sample_operator_base, min_class_type>::value >::type;

	template<class min_class_type>
	using type_enable_if_not_jitter_class = typename enable_if< !is_base_of<matrix_operator_base, min_class_type>::value && !is_base_of<gl_operator_base, min_class_type>::value >::type;

}}


/// A standard interface for flagging serious runtime snafus.
/// At the moment this is hardwired to throw an exception but offers us the ability to
/// change that behavior later or specialize it for certain contexts.
///
/// Because this throws an exception you should **not** call this function in an audio perform routine.
void error(const std::string& description) {
	throw std::runtime_error(description);
}


#include "c74_min_symbol.h"
#include "c74_min_atom.h"
#include "c74_min_map.h"
#include "c74_min_dictionary.h"


namespace c74 { 
namespace min {
	static max::t_class*	this_class = nullptr;
	static bool				this_class_init = false;
}}


#include "c74_min_object_components.h"	// Shared components of Max objects
#include "c74_jitter.h"
#include "c74_min_ports.h"				// Inlets and Outlets
#include "c74_min_method.h"				// Messages to objects
#include "c74_min_attribute.h"			// Attributes of objects
#include "c74_min_logger.h"				// Console / Max Window output
#include "c74_min_operator_perform.h"	// Perform-based MSP object add-ins
#include "c74_min_operator_sample.h"	// Sample-based MSP object add-ins
#include "c74_min_operator_matrix.h"	// Jitter MOP
#include "c74_min_operator_gl.h"		// Jitter GL
#include "c74_min_object_wrapper.h"		// Max wrapper for Min objects
#include "c74_min_object.h"				// The Min object class that glues it all together

#include "c74_min_timer.h"				// Wrapper for clocks
#include "c74_min_buffer.h"				// Wrapper for MSP buffers
#include "c74_min_path.h"				// Wrapper class for accessing the Max path system
#include "c74_min_texteditor.h"			// Wrapper for text editor window

#include "c74_min_accessories.h"		// Library of miscellaneous helper functions and widgets


#define MIN_EXTERNAL(cpp_classname) \
void ext_main (void* r) { \
	c74::min::wrap_as_max_external< cpp_classname > ( #cpp_classname, __FILE__ , r ); \
}

#define MIN_EXTERNAL_CUSTOM(cpp_classname, max_name) \
void ext_main (void* r) { \
	c74::min::wrap_as_max_external< cpp_classname > ( #max_name, __FILE__ , r ); \
}

