
#include "kind.hpp"

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