//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
namespace max {

	MOCK_EXPORT t_ptr sysmem_newptr(long size) { return (t_ptr)malloc(size); }
	MOCK_EXPORT t_ptr sysmem_newptrclear(long size) { t_ptr p = (t_ptr)malloc(size); memset(p, 0, size); return p; }
	MOCK_EXPORT void sysmem_freeptr(void *ptr) { free(ptr); }
	MOCK_EXPORT void sysmem_copyptr(const void *src, void *dst, long bytes) { memcpy(dst, src, bytes); }

}} //namespace c74::max
