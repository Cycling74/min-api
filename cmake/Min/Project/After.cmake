# If min::external is set, propagate to max::external so max-sdk-base's After.cmake
# picks it up and creates the MODULE target with the correct Max SDK configuration.
get_property(
    _is_ext
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    PROPERTY min::external)
if(_is_ext)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY max::external YES)

    # If no explicit source list is set, pre-populate max::sources with a glob that
    # excludes *_test.cpp so the test files don't end up compiled into the module.
    get_property(_explicit_sources DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY max::sources)
    if(NOT _explicit_sources)
        file(GLOB _min_sources CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
            "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
        list(FILTER _min_sources EXCLUDE REGEX "_test\\.cpp$")
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY max::sources "${_min_sources}")
        unset(_min_sources)
    endif()
    unset(_explicit_sources)
endif()

# Delegate to max-sdk-base's After.cmake — checks max::external, creates the MODULE target,
# links Max:: targets, and configures the bundle/suffix/linker flags.
include("${MAXSDK_BASE_DIR}/cmake/Max/Project/After.cmake")

if(NOT _is_ext)
    return()
endif()

set(_tgt "${PROJECT_NAME}")

# Link Min::API — exposes min-api headers and C74_MIN_API define; Max:: propagates transitively
target_link_libraries(${_tgt} PRIVATE Min::API)

# Detect min-lib: prefer FetchContent variable, fall back to filesystem sibling convention
if(DEFINED min-lib_SOURCE_DIR OR EXISTS "${MINAPI_DIR}/../min-lib/include/c74_lib.h")
    target_compile_definitions(${_tgt} PRIVATE C74_USE_MIN_LIB)
endif()

# Xcode warning policy — mirrors old min-posttarget.cmake behavior
option(C74_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
if(APPLE)
    set(_warn_flags "-Wall -Wmissing-field-initializers -Wno-unused-lambda-capture -Wno-unknown-warning-option")
    if(C74_WARNINGS_AS_ERRORS)
        string(APPEND _warn_flags " -Werror")
    endif()
    set_target_properties(${_tgt} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS "${_warn_flags}")
    unset(_warn_flags)
endif()

unset(_tgt)
unset(_is_ext)
