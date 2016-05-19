cmake_minimum_required(VERSION 3.0)
project(MinAPI)

include(${CMAKE_CURRENT_SOURCE_DIR}/script/min-package.cmake)

enable_testing()

SUBDIRLIST(TESTDIRS ${CMAKE_CURRENT_SOURCE_DIR}/test)
foreach(testdir ${TESTDIRS})
	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/${testdir}/CMakeLists.txt")
		message("Generating Unit Test: ${testdir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/${testdir})
		add_test(NAME ${testdir}
		         COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test/${testdir}/${testdir})
 	endif ()
endforeach()
