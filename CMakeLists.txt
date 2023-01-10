# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.19)

option(C74_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

if (${CMAKE_GENERATOR} MATCHES "Xcode")
 	if (${XCODE_VERSION} VERSION_LESS 10)
  		message(STATUS "Xcode 10 or later is required. Please install from the Mac App Store.")
		return ()
	endif ()
endif ()

# Add the max sdk, if it exists
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/max-sdk-base/CMakeLists.txt")
    add_subdirectory(max-sdk-base)
else()
    message(FATAL_ERROR "The max-sdk-base repository is missing. Maybe you did not clone recursively?")
endif ()

include(script/c74_add_min_target.cmake)
include(test/c74_add_min_unit_test.cmake)




project(MinAPI)


if (WIN32)
    add_definitions(
		-DMAXAPI_USE_MSCRT
		-DWIN_VERSION
		-D_USE_MATH_DEFINES
	)
endif ()

file(GLOB_RECURSE MIN_API_HEADERS
	 RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	 "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

set(version 0.6.0)
set(target min-api)

add_library(${target} INTERFACE ${MIN_API_HEADERS})

set_target_properties(${target} PROPERTIES FOLDER "C74_Libraries")
set_target_properties(${target} PROPERTIES VERSION ${version})

target_include_directories(${target} INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/include>"
)

add_library(API ALIAS min-api) # alias for backwards compatibility



# Target for including the test headers
set(target min-api-test-headers)
add_library(${target} INTERFACE)
target_include_directories(${target} INTERFACE test)


enable_testing()

set(C74_MOCK_TARGET_DIR "${CMAKE_CURRENT_LIST_DIR}/../../tests")###
# min-devkit and probably many user projects add the min-api as a subdirectory,
# however, we want the min-api to be able to act as a standalone project as well.
# so projects including the min-api should use the various .cmake scripts instead
# of including min-api as a subdirectory.
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
else()
    message(DEPRECATION "Adding min-api via add_subdirectory() is deprecated and might break your setup in the future. Please remove the add_subdirectory() line from your CMakeLists.txt file.")
    add_subdirectory(test/mock "${CMAKE_BINARY_DIR}/mock")
endif()
set_target_properties(mock_kernel PROPERTIES FOLDER "C74_Libraries")

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
