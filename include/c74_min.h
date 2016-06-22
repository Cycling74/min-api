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
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>

namespace c74 {
namespace min {
	using uchar = unsigned char;
	using sample = double;
	using samples = std::vector<sample>;
}}

#include "c74_min_symbol.h"
#include "c74_min_atom.h"
#include "c74_min_map.h"
#include "c74_min_logger.h"
#include "c74_min_dictionary.h"

namespace c74 { 
namespace min {
	static max::t_class* this_class = nullptr;
}}

#include "c74_min_object_components.h"	// Shared components of Max objects
#include "c74_min_operator_perform.h"	// Perform-based MSP object add-ins
#include "c74_min_operator_sample.h"	// Sample-based MSP object add-ins
#include "c74_min_operator_matrix.h"	// Jitter MOP
#include "c74_min_operator_gl.h"		// Jitter GL
#include "c74_min_object.h"				// Max objects

#include "c74_min_clock.h"				// Wrapper for clocks
#include "c74_min_buffer.h"				// Wrapper for MSP buffers
#include "c74_min_path.h"				// Wrapper class for accessing the Max path system
#include "c74_min_texteditor.h"			// Wrapper for text editor window

#include "c74_min_accessories.h"		// Library of miscellaneous helper functions and widgets


#define MIN_EXTERNAL(cpp_classname) \
void ext_main (void* r) { \
	define_min_external< cpp_classname > ( #cpp_classname, __FILE__ , r ); \
}


#define METHOD( name )								c74::min::method				name = { this, #name , [this](c74::min::atoms& args)
#define ATTRIBUTE( name, type, default)				c74::min::attribute< type >		name = { this, #name , default, [this](c74::min::atoms& args)
#define ATTRIBUTE_READONLY( name, type, default)	c74::min::attribute< type >		name = { this, #name , default, [this](c74::min::atoms& args)
//#define INLET(  name, desc )						c74::min::inlet					name = { this, desc };
//#define OUTLET( name, ... )						c74::min::outlet				name = { this, __VA_ARGS__ };
#define CLOCK( name )								c74::min::clock					name = { this, [this](c74::min::atoms& args)
#define END };

#ifdef WIN_VERSION
#define MIN_CLAMP( in, lo, hi )				c74::max::clamp<std::remove_reference<decltype(in)>::type>(in, lo, hi)
#else
#define MIN_CLAMP( in, lo, hi )				c74::max::clamp<typeof(in)>(in, lo, hi)
#endif
