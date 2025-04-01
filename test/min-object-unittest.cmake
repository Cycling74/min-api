# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.



set(ORIGINAL_NAME "${PROJECT_NAME}")
set(TEST_NAME "${PROJECT_NAME}_test")
#project(${PROJECT_NAME}_test)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.cpp")

	#if (NOT DEFINED SOURCE_FILES)
	#	get_target_property(SOURCE_FILES ${ORIGINAL_NAME} SOURCES)
	#endif ()
	
	c74_test_source_files_macro()

	if (NOT TARGET mock_kernel)
		set(C74_MOCK_TARGET_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../tests")
		add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../test/mock" "${CMAKE_BINARY_DIR}/mock")
	endif ()
	
	c74_add_unit_test(${TEST_NAME} 
		OUTPUT_DIRECTORY 
			"${CMAKE_CURRENT_SOURCE_DIR}/../../../tests"
		SOURCES 
			${TEST_NAME}.cpp ${TEST_SOURCE_FILES}
	)
	 
endif ()
	 