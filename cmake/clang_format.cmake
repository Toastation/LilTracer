
function(add_clang_format_target TARGET_NAME SOURCE_DIR)

	find_program(CLANG-FORMAT_PATH clang-format)

	if(CLANG-FORMAT_PATH) 
		file(GLOB_RECURSE FORMAT_SOURCES
			LIST_DIRECTORIES false
			"${SOURCE_DIR}/*.h" 
			"${SOURCE_DIR}/*.cpp"
			)


		add_custom_target(
			${TARGET_NAME}
			COMMAND ${CLANG-FORMAT_PATH} -i ${FORMAT_SOURCES} --style=WebKit
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			COMMENT "Running clang-format on lt sources"
			)

	endif()

endfunction()