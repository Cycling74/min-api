# Copyright 2020 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

cmake_minimum_required(VERSION 3.20)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

enable_testing()

if (APPLE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.11")
endif ()

project(min-tests)

# Make our .exe and mock_kernel.dll end up in the same directory
set(OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${OUTPUT_DIRECTORY}")
set(C74_MOCK_TARGET_DIR ${OUTPUT_DIRECTORY})

include(FetchContent)
fetchcontent_declare(
    mock
    GIT_REPOSITORY https://github.com/Cycling74/mock.git
    GIT_TAG c4c23c49ceda913b506a0834b59e0ec71d2ec06f
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mock")
fetchcontent_makeavailable(mock)

set(SOURCES atom.cpp limit.cpp main.cpp object.cpp symbol.cpp)

add_executable(min-tests ${SOURCES})

target_compile_definitions(min-tests PUBLIC -DMIN_TEST)

target_include_directories(min-tests PUBLIC
	"${C74_MIN_API_DIR}/include"
	"${MAXSDK_BASE_DIR}/c74support"
	"${MAXSDK_BASE_DIR}/c74support/max-includes"
	"${MAXSDK_BASE_DIR}/c74support/msp-includes"
	"${MAXSDK_BASE_DIR}/c74support/jit-includes"
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(min-tests mock_kernel)

set_target_properties(min-tests PROPERTIES CXX_STANDARD 17)
set_target_properties(min-tests PROPERTIES CXX_STANDARD_REQUIRED ON)

if (APPLE)
	#target_link_libraries(min-tests stdc++ "-framework CoreServices" "-framework CoreFoundation")
	set_target_properties(min-tests PROPERTIES LINK_FLAGS "-Wl,-F'${MAXSDK_BASE_DIR}/c74support/jit-includes', -weak_framework JitterAPI")
	target_compile_options(min-tests PRIVATE -DCATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS)
endif ()

add_test(NAME min-tests COMMAND min-tests)
