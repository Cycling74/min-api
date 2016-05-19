cmake_minimum_required(VERSION 3.0)

enable_testing()

include_directories( 
	"${C74_INCLUDES}"
	"${CMAKE_CURRENT_LIST_DIR}/catch/include"
	"${CMAKE_CURRENT_LIST_DIR}/mock"
)

#set(CMAKE_CXX_FLAGS "-std=c++1y -stdlib=libc++ -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cpp)

add_test(NAME ${PROJECT_NAME}
         COMMAND ${PROJECT_NAME})
