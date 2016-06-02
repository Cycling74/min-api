cmake_minimum_required(VERSION 3.0)

project(test_${PROJECT_NAME})

enable_testing()

include_directories( 
	"${C74_INCLUDES}"
	"${CMAKE_CURRENT_LIST_DIR}/catch/include"
	"${CMAKE_CURRENT_LIST_DIR}/mock"
)

#set(CMAKE_CXX_FLAGS "-std=c++1y -stdlib=libc++ -fprofile-arcs -ftest-coverage")
#set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
#set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

target_link_libraries(${PROJECT_NAME} "mock_kernel")

if (APPLE)
	target_link_libraries(${PROJECT_NAME} "-weak_framework JitterAPI")
	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-F${C74_MAX_API_DIR}/lib/mac")
endif ()
if (WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD	# Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different		# which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../tests/mock_kernel.dll"      # <--this is in-file
        ${CMAKE_CURRENT_BINARY_DIR}                 # <--this is out-file path
)
endif ()

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
