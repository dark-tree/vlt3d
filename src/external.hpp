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
#include <algorithm>

#include "stb/image/read.h"

#include <shaderc/shaderc.hpp>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

// usage hints
#define READONLY
