
## GLAD

add_library(GLAD STATIC
    glad/glad.c
)

target_link_libraries(GLAD
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
)

## STB Image

add_library(STB_IMAGE STATIC
	stb/image.c
)

target_link_libraries(STB_IMAGE
	# Nothing to link with
)

add_subdirectory(glfw)

set(SKIP_SPIRV_TOOLS_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_INSTALL ON)
add_subdirectory(shaderc)
