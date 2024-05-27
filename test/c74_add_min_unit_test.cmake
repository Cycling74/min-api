# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.



# Add a unit test target with given name from given sources files. Certain min/max-specific 
# configurations are applied to the test target. # 
#
# Call example:
#
# c74_add_unit_test(my_random_test 
#     SOURCES 
#         random.cpp random_test.cpp 
#     OUTPUT_DIRECTORY 
#         ../tests
# )

function(c74_add_unit_test target)
	set(options LINK_TO_C74_LIBS)
	set(oneValueArgs OUTPUT_DIRECTORY)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}")
	
	c74_max_pre_project_calls()
	c74_max_post_project_calls()
	add_definitions(-DC74_MIN_API)
	add_definitions(-DC74_USE_MIN_LIB)

	c74_add_unit_test_impl(${target} 
		OUTPUT_DIRECTORY 
			"${PARAMS_OUTPUT_DIRECTORY}"
		MAX_SDK_JIT_INCLUDES
			${MAX_SDK_JIT_INCLUDES}  # these are initialized in the c74_pre_project_calls()
		SOURCES 
			${PARAMS_SOURCES}
		DO_LINK_TO_C74_LIBS
			${PARAMS_LINK_TO_C74_LIBS}
	)
endfunction()



# Auto discover a test for an already existing target. 
#
# This function simulates the min-object-unittest.cmake script. It is expected that a source file with the name 
# [target].cpp and a test file with the name [target]_test.cpp exist. If the test file does not exist, this function
# does nothing

function(c74_add_auto_unit_test target)
	

	get_target_property(SOURCE_FILES ${target} SOURCES)
	
	if (NOT SOURCE_FILES)
		message(ERROR "No source files given")
	endif ()
	
	set(ORIGINAL_NAME "${target}")
	set(TEST_NAME "${target}_test")

	if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.cpp")
		c74_test_source_files_macro()

		#if (NOT TARGET mock_kernel)
		#	set(C74_MOCK_TARGET_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../tests")###
		#	#add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../../min-api/test/mock" "${CMAKE_BINARY_DIR}/mock")###
		#endif ()
		
		c74_add_unit_test(${TEST_NAME} 
			OUTPUT_DIRECTORY 
				"${CMAKE_CURRENT_SOURCE_DIR}/../../../tests"
			SOURCES 
				${TEST_NAME}.cpp ${TEST_SOURCE_FILES}
		)
	endif ()
endfunction()


macro(c74_test_source_files_macro)
	set(TEST_SOURCE_FILES "")
	FOREACH(SOURCE_FILE ${SOURCE_FILES})
		set(ORIGINAL_WITH_EXT "${ORIGINAL_NAME}.cpp")
		if (SOURCE_FILE STREQUAL ORIGINAL_WITH_EXT)
			set(TEST_SOURCE_FILES ${TEST_SOURCE_FILES} ${TEST_NAME}.cpp)
		else()
			if(NOT SOURCE_FILE MATCHES ".+\.rc") # omit resource files
				set(TEST_SOURCE_FILES "${TEST_SOURCE_FILES}" ${SOURCE_FILE})
			endif()
		endif()
	ENDFOREACH()
endmacro()



function(c74_add_unit_test_impl target)
	set(oneValueArgs OUTPUT_DIRECTORY MAX_SDK_JIT_INCLUDES DO_LINK_TO_C74_LIBS)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 PARAMS "${options}" "${oneValueArgs}" "${multiValueArgs}")

	enable_testing()
	
	# set(CMAKE_CXX_FLAGS "-fprofile-arcs -ftest-coverage")
	# set(CMAKE_C_FLAGS "-fprofile-arcs -ftest-coverage")
	# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PARAMS_OUTPUT_DIRECTORY}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

	add_executable(${target} ${PARAMS_SOURCES})
	add_dependencies(${target} mock_kernel)
	target_compile_definitions(${target} PUBLIC -DMIN_TEST)
	
	set_target_properties(${target} PROPERTIES FOLDER "Unit Tests")
	set_target_properties(${target} PROPERTIES CXX_STANDARD 17)
	set_target_properties(${target} PROPERTIES CXX_STANDARD_REQUIRED ON)

	target_link_libraries(${target} PUBLIC mock_kernel)
	target_link_libraries(${target} PRIVATE max-sdk-base-headers min-api min-api-test-headers)
	
	if (PARAMS_DO_LINK_TO_C74_LIBS)
		target_link_libraries(${target} PRIVATE max-sdk-base)
	endif ()

	if (APPLE)
		set_target_properties(${target} PROPERTIES LINK_FLAGS "-Wl,-F'${PARAMS_MAX_SDK_JIT_INCLUDES}', -weak_framework JitterAPI")
		target_compile_options(${target} PRIVATE -DCATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS)
	endif ()
	if (WIN32)
		set_target_properties(${target} PROPERTIES COMPILE_PDB_NAME ${target})
		
		##target_link_libraries(${target} PUBLIC ${MaxAPI_LIB})
		##target_link_libraries(${target} PUBLIC ${MaxAudio_LIB})
		##target_link_libraries(${target} PUBLIC ${Jitter_LIB})
	endif ()

	add_test(NAME ${target} COMMAND ${target})
endfunction()