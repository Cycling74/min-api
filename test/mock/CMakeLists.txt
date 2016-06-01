cmake_minimum_required(VERSION 3.0)

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../min-api)


#############################################################
project(mock_kernel)
#############################################################


include_directories( 
	"${C74_MIN_API_DIR}/max-api/include"
	"."
)


add_definitions(
	-D_USE_MATH_DEFINES
)


set(CMAKE_MACOSX_RPATH 1)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../../../tests")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")


add_library( 
	${PROJECT_NAME} 
	SHARED
	c74_mock.cpp
)


set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

