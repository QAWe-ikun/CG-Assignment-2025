#pragma once

#include <glm/glm.hpp>

namespace render
{
	enum class AntialiasMode
	{
		None,
		FXAA,
		MLAA,
		SMAA
	};

	struct CameraMatrices
	{
		glm::mat4 view_matrix;
		glm::mat4 proj_matrix;
		glm::mat4 prev_view_proj_matrix;
		glm::vec3 eye_position;
	};

	struct PrimaryLightParams
	{
		glm::vec3 direction;
		glm::vec3 intensity;
	};

	struct AmbientParams
	{
		glm::vec3 intensity = glm::vec3(50);
		float ao_radius = 70.0;
		float ao_blend_ratio = 0.017;
		float ao_strength = 1.0;
	};

	struct BloomParams
	{
		float bloom_attenuation = 1.2;
		float bloom_strength = 0.025;
	};

	struct ShadowParams
	{
		float csm_linear_blend = 0.56;
	};

	struct SkyParams
	{
		float brightness;
		float turbidity = 2.0;
		float brightness_mult = 0.1;
	};

	struct FunctionMask
	{
		bool ssgi = true;
		bool use_bloom_mask = true;
	};

	struct Params
	{
		AntialiasMode aa_mode = AntialiasMode::MLAA;
		CameraMatrices camera;
		PrimaryLightParams primary_light;
		AmbientParams ambient = {};
		BloomParams bloom = {};
		ShadowParams shadow = {};
		SkyParams sky = {};
		FunctionMask function_mask = {};
	};
}