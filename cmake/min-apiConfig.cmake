# min-apiConfig.cmake
# Supports find_package(min-api) after the package is on CMAKE_PREFIX_PATH,
# e.g. when FetchContent_MakeAvailable sets up the paths automatically in CMake 3.24+.

get_filename_component(_minapi_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT DEFINED MINAPI_DIR)
    set(MINAPI_DIR "${_minapi_dir}" CACHE PATH "Root of min-api")
endif()

list(APPEND CMAKE_MODULE_PATH "${_minapi_dir}/cmake")

# Ensure max-sdk-base is available — Min::API depends on Max:: targets
if(NOT DEFINED MAXSDK_BASE_DIR)
    find_package(max-sdk-base QUIET)
    if(NOT max-sdk-base_FOUND)
        include(FetchContent)
        fetchcontent_declare(
            max-sdk-base
            GIT_REPOSITORY https://github.com/Cycling74/max-sdk-base.git
            GIT_TAG f3e555a5069c0efecc53135da30994ccbb869f90)
        fetchcontent_makeavailable(max-sdk-base)
    endif()
endif()

# Ensure readerwriterqueue is available — included unconditionally in c74_min_api.h
if(NOT DEFINED readerwriterqueue_SOURCE_DIR)
    include(FetchContent)
    fetchcontent_declare(
        readerwriterqueue
        GIT_REPOSITORY https://github.com/cameron314/readerwriterqueue.git
        GIT_TAG a500a29fe07c759fa793a95f4782645088ce5b7a
        SOURCE_DIR "${_minapi_dir}/include/readerwriterqueue")
    fetchcontent_makeavailable(readerwriterqueue)
endif()

include("${_minapi_dir}/cmake/Min/Targets.cmake")

unset(_minapi_dir)
