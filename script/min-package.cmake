# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.

include(FetchContent)

fetchcontent_declare(
    max-sdk-base
    GIT_REPOSITORY https://github.com/Cycling74/max-sdk-base.git
    GIT_TAG f3e555a5069c0efecc53135da30994ccbb869f90)
fetchcontent_makeavailable(max-sdk-base)

# Header-only queue library; SOURCE_DIR keeps it at include/readerwriterqueue
# so existing "#include <readerwriterqueue/readerwriterqueue.h>" includes keep working.
fetchcontent_declare(
    readerwriterqueue
    GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
    GIT_TAG a500a29fe07c759fa793a95f4782645088ce5b7a
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../include/readerwriterqueue")
fetchcontent_makeavailable(readerwriterqueue)

include(${MAXSDK_BASE_DIR}/script/max-package.cmake)
