//	Copyright 2013 - Cycling '74
//	Timothy Place, tim@cycling74.com	

#pragma once

namespace c74 {
	namespace max {
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
			//printf(msg);
			std::cout << msg;
		}
		
	}
}

