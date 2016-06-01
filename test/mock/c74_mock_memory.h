//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
namespace max {

t_ptr sysmem_newptr(long size) { return (t_ptr)malloc(size); }
t_ptr sysmem_newptrclear(long size) { t_ptr p = (t_ptr)malloc(size); memset(p, 0, size); return p; }
void sysmem_freeptr(void *ptr) { free(ptr); }
void sysmem_copyptr(const void *src, void *dst, long bytes) { memcpy(dst, src, bytes); }

}} //namespace c74::max
