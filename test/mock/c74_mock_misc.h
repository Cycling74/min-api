//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#ifndef __MOCK_MISC_H__
#define __MOCK_MISC_H__


/**	Post to the conole, mocking the cpost() function in the Max kernel.
	This version is slightly simpler to minimize dependencies on the OS.
 */
inline void cpost(const char *fmt, ...)
{
	char msg[2048+2];
	va_list ap;
	
	va_start(ap, fmt);
	vsnprintf(msg, 2048, fmt, ap);
	va_end(ap);
	msg[2048] = '\0';
	printf(msg);
}


inline char *strncpy_zero(char *dst, const char* src, long size) { return strncpy(dst, src, size); }


#endif // __MOCK_MISC_H__
