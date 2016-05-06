# Copyright (c) 2016, Cycling '74
# Usage of this file and its contents is governed by the MIT License

set(C74_MAX_API_DIR ${CMAKE_CURRENT_LIST_DIR}/../max-api)
include(${C74_MAX_API_DIR}/script/max-pretarget.cmake)

set(C74_INCLUDES "${C74_MAX_API_DIR}/include" "${CMAKE_CURRENT_LIST_DIR}/../include")
file(GLOB_RECURSE C74_MIN_HEADERS ${CMAKE_CURRENT_LIST_DIR}/../include/*.h)
