cmake_minimum_required(VERSION 3.0)
project(MinAPI)

include(${CMAKE_CURRENT_SOURCE_DIR}/script/min-package.cmake)


file(GLOB_RECURSE MIN_API_HEADERS
	 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_custom_target( API ALL
	SOURCES ${MIN_API_HEADERS}
)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/mock)


enable_testing()

SUBDIRLIST(TESTDIRS ${CMAKE_CURRENT_SOURCE_DIR}/test)
foreach(testdir ${TESTDIRS})
	if (${testdir} STREQUAL "mock")
	else ()
		if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/${testdir}/CMakeLists.txt")
			message("Generating Unit Test: ${testdir}")
			add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/${testdir})
	 	endif ()
	endif()
endforeach()
