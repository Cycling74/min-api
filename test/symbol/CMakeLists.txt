cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

include(${CMAKE_CURRENT_SOURCE_DIR}/../UnitTest.cmake)

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
