/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_max.h"
#include "c74_msp.h"
#include <array>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace c74 {
namespace min {
	using uchar = unsigned char;
	using sample = double;
}}

#include "c74_min_symbol.h"
#include "c74_min_atom.h"
#include "c74_min_map.h"
#include "c74_min_dictionary.h"

namespace c74 { 
namespace min {
	static max::t_class* this_class = nullptr;
}}

#include "c74_min_object.h"
#include "c74_min_object_audio.h"
#include "c74_min_object_matrix.h"

#include "c74_min_clock.h"
#include "c74_min_buffer.h"
#include "c74_min_path.h"
#include "c74_min_texteditor.h"

#include "c74_min_accessories.h"


#define MIN_EXTERNAL(cpp_classname) \
void ext_main (void* r) { \
	define_min_external< cpp_classname > ( #cpp_classname, __FILE__ , r ); \
}


#define METHOD( name )						c74::min::method				name = { this, #name , [this](c74::min::atoms& args)
#define ATTRIBUTE( name, type, default )	c74::min::attribute< type >		name = { this, #name , default, [this](c74::min::atoms& args)
#define INLET(  name, desc )				c74::min::inlet					name = { this, desc };
#define OUTLET( name, ... )					c74::min::outlet				name = { this, __VA_ARGS__ };
#define CLOCK( name )						c74::min::clock					name = { this, [this](c74::min::atoms& args)
#define END };

#define MIN_CLAMP( in, lo, hi )				c74::max::clamp<typeof(in)>(in, lo, hi)

