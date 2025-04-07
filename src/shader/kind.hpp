
#pragma once

#include "external.hpp"

/**
 * Java-style enum representing the shader type,
 * combines the vulkan and ShaderC shader type enums
 */
class Kind {

	public:

		static const Kind VERTEX;
		static const Kind FRAGMENT;
		static const Kind COMPUTE;
		static const Kind GEOMETRY;
		static const Kind TESSELLATION_CONTROL;
		static const Kind TESSELLATION_EVALUATION;
		static const Kind RAYGEN;
		static const Kind ANYHIT;
		static const Kind CLOSEST;
		static const Kind MISS;
		static const Kind INTERSECTION;
		static const Kind CALLABLE;
		static const Kind TASK;
		static const Kind MESH;

	public:

		// https://github.com/google/shaderc/blob/f9eb1c730c20c5fa7aab81cf7074c22c6342274f/libshaderc/include/shaderc/shaderc.h#L36C15-L90
		const shaderc_shader_kind shaderc;

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkShaderStageFlagBits.html#_c_specification
		const VkShaderStageFlags vulkan;

		inline constexpr Kind(shaderc_shader_kind shaderc, VkShaderStageFlags stage)
		: shaderc(shaderc), vulkan(stage) {}

		Kind operator | (Kind other) const {
			return {other.shaderc, other.vulkan | vulkan};
		}

};

