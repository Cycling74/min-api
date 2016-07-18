cmake_minimum_required(VERSION 3.0)
project(MinAPI)


if (${CMAKE_GENERATOR} MATCHES "Xcode")
 		if (${XCODE_VERSION} VERSION_LESS 7)
  			message(STATUS "Xcode 7 is required. Please install from the Mac App Store.")
			return ()
		endif ()
endif ()


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





find_package (Doxygen)
option (BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if (BUILD_DOCUMENTATION)
    if (NOT DOXYGEN_FOUND)
         message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif ()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    message("Doxygen build started.")

    add_custom_target(doc
                      COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
                      COMMENT "Generating API documentation with Doxygen"
                      VERBATIM)

    #    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION     share/doc)
endif ()