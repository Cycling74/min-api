/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "readerwriterqueue/readerwriterqueue.h"

namespace c74 {
namespace min {
	
	template<typename T>
	using fifo = moodycamel::ReaderWriterQueue<T>;
	
}} // namespace c74::min
