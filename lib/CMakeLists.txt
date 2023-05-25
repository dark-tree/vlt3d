
add_subdirectory(glfw)
add_subdirectory(glm)

set(SKIP_SPIRV_TOOLS_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_INSTALL ON)
add_subdirectory(shaderc)
