# Copyright (c) 2016, Cycling '74
# Usage of this file and its contents is governed by the MIT License

if (${CMAKE_GENERATOR} MATCHES "Xcode")
 		if (${XCODE_VERSION} VERSION_LESS 7)
  			message(STATUS "Xcode 7 or 8 is required. Please install from the Mac App Store")
			return ()
		endif ()
endif ()


set(C74_MAX_API_DIR ${CMAKE_CURRENT_LIST_DIR}/../max-api)
include(${C74_MAX_API_DIR}/script/max-pretarget.cmake)

set(C74_INCLUDES "${C74_MAX_API_DIR}/include" "${CMAKE_CURRENT_LIST_DIR}/../include")
file(GLOB_RECURSE C74_MIN_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../include/*.h)


if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../min-lib")
    message(STATUS "Min-Lib found")
	add_definitions(
		-DC74_USE_MIN_LIB
	)
endif()