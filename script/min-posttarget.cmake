# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

include(${C74_MAX_API_DIR}/script/max-posttarget.cmake)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if (APPLE)
    # enforce a strict warning policy
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS "-Wall -Werror -Wmissing-field-initializers -Wno-unused-lambda-capture -Wno-unknown-warning-option")    
    # -Wmost -Wno-four-char-constants -Wno-unknown-pragmas $(inherited)
endif ()
