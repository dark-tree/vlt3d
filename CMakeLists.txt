cmake_minimum_required(VERSION 3.10)
project(VLT3D)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE, Debug)

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenAL REQUIRED)
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
		lib/msdf/
)

file(GLOB_RECURSE VLT3D_SOURCES RELATIVE
		${CMAKE_SOURCE_DIR}
		"src/*.cpp"
)

add_library(external
		lib/implementation.cpp
)

add_executable(main
		${VLT3D_SOURCES}
)

target_link_libraries(main
		OpenAL::OpenAL
		Threads::Threads
		vulkan
		glfw
		shaderc
		msdfgen::msdfgen-core
		msdfgen::msdfgen-ext
		external
)

target_compile_options(external PRIVATE -Wno-volatile)
target_compile_options(main PRIVATE -Wno-volatile -Wall)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
