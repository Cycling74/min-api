cmake_minimum_required(VERSION 3.19)

project(MinExample)

set(AUTHOR_DOMAIN "com.cycling74")
set(COPYRIGHT_STRING "Copyright (c) 2020 Cycling '74")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

#don't pollute the source directory
set(C74_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/externals/" CACHE FILEPATH "The destination path for the built external")
set(CONAN_MIN-API_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../)

include(${CONAN_MIN-API_ROOT}/script/min-package.cmake)
include(${CONAN_MIN-API_ROOT}/script/min-pretarget.cmake)

include_directories("${C74_INCLUDES}")
set(SOURCE_FILES "min.example.cpp")
add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})

include(${CONAN_MIN-API_ROOT}/script/min-posttarget.cmake)

set_target_properties(min.example PROPERTIES SUFFIX ".mxl_${CMAKE_SYSTEM_PROCESSOR}")
target_link_libraries(min.example PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/MaxAudioAPI.so
  ${CMAKE_CURRENT_LIST_DIR}/libJitterAPI.so
  )
