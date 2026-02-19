# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

include(FetchContent)
FetchContent_Declare(
    max-sdk-base
    GIT_REPOSITORY https://github.com/Cycling74/max-sdk-base.git
    GIT_TAG        f3e555a5069c0efecc53135da30994ccbb869f90
)
FetchContent_MakeAvailable(max-sdk-base)

include(${MAXSDK_BASE_DIR}/script/max-package.cmake)
