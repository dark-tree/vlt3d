#pragma once

// gotta go fast
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_SIMD_AVX2

// external
#include <stb/image/read.h>
#include <stb/image/write.h>
#include <glad/glad.h>
#include <winx/winx.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define glfwTerminate dont_use_glfw_terminate_because_it_causes_segfault_on_my_driver_thank_you

// C++
#include <sstream>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <ctime>
#include <tuple>
#include <unordered_map>
#include <chrono>
#include <queue>
#include <functional>
#include <mutex>
#include <thread>
#include <random>
#include <regex>
#include <utility>
#include <filesystem>
#include <condition_variable>
#include <fstream>
#include <list>

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#ifdef WIN32
//#	define NOMINMAX
//#	include <windows.h>
//#	include <direct.h>
//#else
//#	include <unistd.h>
//#	include <sys/stat.h>
//#endif

