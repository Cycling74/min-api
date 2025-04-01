# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

include("${CMAKE_CURRENT_LIST_DIR}/c74_set_target_xcode_warning_flags.cmake")
include(${C74_MAX_SDK_DIR}/script/max-posttarget.cmake)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

c74_set_target_xcode_warning_flags(${PROJECT_NAME})
