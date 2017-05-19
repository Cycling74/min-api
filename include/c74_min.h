/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#define C74_MIN_WITH_IMPLEMENTATION

#include "c74_min_api.h"
#include "c74_min_impl.h"
#include "c74_min_attribute_impl.h"
#include "c74_min_buffer_impl.h"

#if __has_include("../../min-lib/include/c74_lib.h")
#include "../../min-lib/include/c74_lib.h"
#endif

#undef C74_MIN_WITH_IMPLEMENTATION
