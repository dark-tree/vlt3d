#pragma once

#include "config.hpp"
#include "system.hpp"

// C libs
#include <cstring>

// C++ libs
#include <iostream>
#include <sstream>
#include <vector>
#include <exception>
#include <optional>
#include <array>
#include <functional>
#include <set>
#include <list>
#include <algorithm>
#include <filesystem>

// external libs
#define STB_VORBIS_HEADER_ONLY
#include <AL/al.h>
#include <AL/alc.h>
#include "stb_vorbis.c"
#include "stb_image.h"
#include "stb_image_write.h"
#include <shaderc/shaderc.hpp>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION 1000000
#include "vma/include/vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// usage hints
#define READONLY
