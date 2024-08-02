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
#include <unordered_set>
#include <list>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>
#include <algorithm>
#include <filesystem>
#include <random>
#include <regex>
#include <fstream>

// STB
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"
#include "stb_image.h"
#include "stb_image_write.h"

// ShaderC
#include <shaderc/shaderc.hpp>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// VMA
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION 1000000
#include "vk_mem_alloc.h"

// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

// Lib Format
#include "tt.hpp"
#include "bt.hpp"

// Perlin noise
#include "PerlinNoise.hpp"

// usage hints
#define READONLY
#define NULLABLE
