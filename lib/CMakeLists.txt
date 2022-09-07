
## WINX

add_library(WINX STATIC
    winx/winx.c
)

target_link_libraries(WINX
	${X11_LIBRARIES}
	${X11_Xcursor_LIB}
)

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

