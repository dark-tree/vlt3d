
get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

message("-- Found GLPH example/test directory '${name}'")

file(GLOB_RECURSE SOURCES RELATIVE
	${CMAKE_CURRENT_SOURCE_DIR}
	"*.cpp"
)

add_executable(${name}
	${SOURCES}
)

target_link_libraries(${name}
	GLPH
)

set_target_properties(${name}
	PROPERTIES OUTPUT_NAME main
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
