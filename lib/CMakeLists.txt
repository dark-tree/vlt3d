
add_library(WINX STATIC
    winx/winx.c
)

target_link_libraries(WINX
	${X11_LIBRARIES}
)

add_library(GLAD STATIC
    glad/glad.c
)

target_link_libraries(GLAD
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
)

