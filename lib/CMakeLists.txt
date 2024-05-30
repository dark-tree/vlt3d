
add_subdirectory(glfw)
add_subdirectory(glm)

set(SKIP_SPIRV_TOOLS_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_INSTALL ON)
add_subdirectory(shaderc)

# why is this needed? not sure honestly but i can't
# get this library to compile without this, if someone happens to
# know how to use msdfgen without this please tell
file(GENERATE OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/msdf/msdfgen/msdfgen-config.h" CONTENT "")

set(MSDFGEN_DISABLE_SVG ON)
set(MSDFGEN_CORE_ONLY OFF)
set(MSDFGEN_BUILD_STANDALONE OFF)
set(MSDFGEN_USE_CPP11 ON)
set(MSDFGEN_USE_SKIA OFF)
set(MSDFGEN_INSTALL OFF)
set(MSDFGEN_USE_VCPKG OFF)
add_subdirectory(msdf)