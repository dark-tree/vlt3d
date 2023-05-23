
FILE(GLOB examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(example ${examples})
	if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${example})
		add_subdirectory(${example})
	endif()
endforeach()

