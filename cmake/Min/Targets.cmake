include_guard(GLOBAL)

# Derive MINAPI_DIR from this file's location (cmake/Min/ -> ../../)
get_filename_component(_minapi_dir "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
if(NOT DEFINED MINAPI_DIR)
    set(MINAPI_DIR "${_minapi_dir}" CACHE PATH "Root of min-api")
endif()
unset(_minapi_dir)

# Ensure Max:: targets exist before we link against them
if(NOT TARGET Max::Max)
    include("${MAXSDK_BASE_DIR}/cmake/Max/Targets.cmake")
endif()

# Min::API — header-only interface target for the min-api layer
add_library(Min::API INTERFACE IMPORTED GLOBAL)
target_include_directories(Min::API INTERFACE "${MINAPI_DIR}/include")
target_compile_definitions(Min::API INTERFACE C74_MIN_API)
# Max:: targets supply Max/MSP/Jitter headers, platform link flags, and Windows defines transitively
target_link_libraries(Min::API INTERFACE Max::Max Max::MSP Max::Jitter)
