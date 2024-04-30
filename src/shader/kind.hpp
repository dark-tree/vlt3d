
#pragma once

#include "external.hpp"

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
		const VkShaderStageFlagBits vulkan;

		constexpr Kind(shaderc_shader_kind kind, VkShaderStageFlagBits stage)
		: shaderc(kind), vulkan(stage) {}

};

const Kind Kind::VERTEX = {shaderc_vertex_shader, VK_SHADER_STAGE_VERTEX_BIT};
const Kind Kind::FRAGMENT = {shaderc_fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT};
const Kind Kind::COMPUTE = {shaderc_compute_shader, VK_SHADER_STAGE_COMPUTE_BIT};
const Kind Kind::GEOMETRY = {shaderc_geometry_shader, VK_SHADER_STAGE_GEOMETRY_BIT};
const Kind Kind::TESSELLATION_CONTROL = {shaderc_tess_control_shader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT};
const Kind Kind::TESSELLATION_EVALUATION = {shaderc_tess_evaluation_shader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT};

// from VK_KHR_ray_tracing_pipeline
const Kind Kind::RAYGEN {shaderc_raygen_shader, VK_SHADER_STAGE_RAYGEN_BIT_KHR};
const Kind Kind::ANYHIT {shaderc_anyhit_shader, VK_SHADER_STAGE_ANY_HIT_BIT_KHR};
const Kind Kind::CLOSEST {shaderc_closesthit_shader, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
const Kind Kind::MISS {shaderc_miss_shader, VK_SHADER_STAGE_MISS_BIT_KHR};
const Kind Kind::INTERSECTION {shaderc_intersection_shader, VK_SHADER_STAGE_INTERSECTION_BIT_KHR};
const Kind Kind::CALLABLE {shaderc_callable_shader, VK_SHADER_STAGE_CALLABLE_BIT_KHR};
