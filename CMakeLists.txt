
project(GLPH)

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE, Debug)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
#find_package(X11 REQUIRED)
#find_package(Wayland REQUIRED)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Select another Build Directory!")
endif()

if (CMAKE_SOURCE_DIR MATCHES " ")
	message(WARNING "Source Directory path contains spaces!")
endif()

if (CMAKE_BINARY_DIR MATCHES " ")
	message(WARNING "Build Directory path contains spaces!")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -g")

add_subdirectory (lib)

include_directories(
	lib/
	src/
	lib/stb/
	lib/glm/
)

file(GLOB_RECURSE GLPH_SOURCES RELATIVE
	${CMAKE_SOURCE_DIR}
	"src/*.cpp"
)

add_library(GLPH STATIC
    ${GLPH_SOURCES}
)

target_link_libraries(GLPH
	Threads::Threads
	vulkan
	glfw
	shaderc
)

file(GLOB_RECURSE XENON_SOURCES RELATIVE
	${CMAKE_SOURCE_DIR}
	"src/*.cpp"
)

add_subdirectory (test)
