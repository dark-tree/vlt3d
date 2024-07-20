cmake_minimum_required(VERSION 3.20)

project(VLT3D)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(FETCHCONTENT_QUIET OFF)

find_package(Threads REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenAL)

# Try: cmake . -B build && cmake --build build/
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
	message(FATAL_ERROR "In source building is deprecated, select another build directory!")
endif()

if (NOT OpenAL_FOUND)
	message(NOTICE "OpenAL was NOT found, falling back to OpenAL-Soft!")

	FetchContent_Declare(
			openal-soft
			GIT_REPOSITORY https://github.com/kcat/openal-soft
			GIT_TAG 1.23.1
			GIT_PROGRESS TRUE
	)

	FetchContent_MakeAvailable(openal-soft)
endif()

FetchContent_Declare(
		glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG 3.4
		GIT_PROGRESS TRUE
)

FetchContent_Declare(
		vma
		GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
		GIT_TAG v3.0.1
		GIT_PROGRESS TRUE
)

FetchContent_Declare(
		stb
		GIT_REPOSITORY https://github.com/nothings/stb
		GIT_TAG 5736b15f7ea0ffb08dd38af21067c314d6a3aae9
		GIT_PROGRESS TRUE
)

FetchContent_Declare(
		glm
		GIT_REPOSITORY https://github.com/g-truc/glm
		GIT_TAG 0.9.9.8
		GIT_PROGRESS TRUE
)

FetchContent_Declare(
		shaderc
		GIT_REPOSITORY https://github.com/google/shaderc
		GIT_TAG v2023.4
		GIT_PROGRESS TRUE
)

# Time for some mental gymnastics for shaderc ...
FetchContent_Populate(shaderc)
message("Fetching external dependencies for target 'shaderc'...")

# Invoke their very special dependency python script
execute_process(
		COMMAND python3 ${shaderc_SOURCE_DIR}/utils/git-sync-deps
		COMMAND_ECHO STDOUT # TODO this doesn't work (no real-time output)
		WORKING_DIRECTORY ${shaderc_SOURCE_DIR}
		RESULT_VARIABLE GIT_SYNC_DEPS_RESULT
)

if(NOT GIT_SYNC_DEPS_RESULT EQUAL 0)
	message(FATAL_ERROR "Failed to sync Shaderc dependencies")
endif()

# Set Shaderc specific options
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)

# ... And finally add shaderc!
add_subdirectory(${shaderc_SOURCE_DIR} ${shaderc_BINARY_DIR})
FetchContent_MakeAvailable(glfw vma stb glm)

include_directories(
		src/
		${glfw_SOURCE_DIR}/include               # Headers in ./GLFW
		${vma_SOURCE_DIR}/include                # Headers in ./
		${stb_SOURCE_DIR}                        # Headers in ./
		${glm_SOURCE_DIR}                        # Headers in ./glm
		${shaderc_SOURCE_DIR}/libshaderc/include # Headers in ./shaderc
		${Vulkan_INCLUDE_DIRS}                   # Headers in ./vulcan
		${OpenAL_INCLUDE_DIRS}                   # Headers in ./AL
)

file(GLOB_RECURSE VLT3D_SOURCES
		"src/*.cpp"
)

add_library(external
		lib/stb_image.cpp
		lib/stb_image_write.cpp
		lib/stb_vorbis.cpp
		lib/vk_mem_alloc.cpp
)

add_executable(main
		${VLT3D_SOURCES}
)

target_link_libraries(main
		OpenAL::OpenAL
		Threads::Threads
		Vulkan::Vulkan
		glfw
		shaderc
		external
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

	# GCC debugging doodads
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		message(STATUS "Compiling with debugging symbols")
		target_compile_options(main PRIVATE -g)
	endif()

	target_compile_options(external PRIVATE -Wno-volatile)
	target_compile_options(main PRIVATE -Wno-volatile -Wall -Werror=return-type)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

	# we4715 - treat missing return statement as error
	target_compile_options(main PRIVATE /W4 /we4715)
endif()

# Copy assets if any exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()