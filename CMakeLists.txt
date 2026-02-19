# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.19)
project(MinAPI)

set(MINAPI_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE PATH "Root of min-api" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


if (${CMAKE_GENERATOR} MATCHES "Xcode")
 		if (${XCODE_VERSION} VERSION_LESS 10)
  			message(STATUS "Xcode 10 or later is required. Please install from the Mac App Store.")
			return ()
		endif ()
endif ()

if (WIN32)
    add_definitions(
		-DMAXAPI_USE_MSCRT
		-DWIN_VERSION
		-D_USE_MATH_DEFINES
	)
endif ()

file(GLOB_RECURSE MIN_API_HEADERS
	 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_custom_target( API ALL
	SOURCES ${MIN_API_HEADERS}
)



enable_testing()

# min-devkit and probably many user projects add the min-api as a subdirectory,
# however, we want the min-api to be able to act as a standalone project as well.
# so projects including the min-api should use the various .cmake scripts instead
# of including min-api as a subdirectory.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    include(FetchContent)
    FetchContent_Declare(
        max-sdk-base
        GIT_REPOSITORY https://github.com/Cycling74/max-sdk-base.git
        GIT_TAG        f3e555a5069c0efecc53135da30994ccbb869f90
    )
    FetchContent_MakeAvailable(max-sdk-base)
    FetchContent_Declare(
        readerwriterqueue
        GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
        GIT_TAG        a500a29fe07c759fa793a95f4782645088ce5b7a
        SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/include/readerwriterqueue"
    )
    FetchContent_MakeAvailable(readerwriterqueue)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()

if(TARGET Max::Max)
    include(Min/Targets)
endif()

find_package (Doxygen QUIET)
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
