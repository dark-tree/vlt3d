
project(GLPH)

cmake_minimum_required(VERSION 3.10)

find_package(OpenGL REQUIRED)
find_package(OpenCL REQUIRED)
find_package(X11 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Please select another Build Directory!")
endif()

if (CMAKE_SOURCE_DIR MATCHES " ")
	message(WARNING "Your Source Directory contains spaces.")
endif()

if (CMAKE_BINARY_DIR MATCHES " ")
	message(WARNING "Your Build Directory contains spaces.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

add_subdirectory (lib)

include_directories(
	lib/
	src/
)

file(GLOB_RECURSE GLPH_SOURCES RELATIVE
	${CMAKE_SOURCE_DIR}
	"src/*.cpp"
)

add_library(GLPH STATIC
    ${GLPH_SOURCES}
)

target_link_libraries(GLPH
	WINX
	GLAD
)

add_subdirectory (demo)

