# Copyright (c) 2016, Cycling '74
# Usage of this file and its contents is governed by the MIT License

include(${C74_MAX_API_DIR}/script/max-posttarget.cmake)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
