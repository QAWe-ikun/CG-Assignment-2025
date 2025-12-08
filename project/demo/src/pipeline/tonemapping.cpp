#include "pipeline/tonemapping.hpp"
#include "asset/shader/tonemapping.frag.hpp"
#include "gpu/graphics-pipeline.hpp"

#include "gpu/sampler.hpp"
#include "util/as-byte.hpp"
#include <SDL3/SDL_gpu.h>

namespace pipeline
{
	std::expected<Tonemapping, util::Error> Tonemapping::create(
		SDL_GPUDevice* device,
		SDL_GPUTextureFormat target_format
	) noexcept
	{
		const gpu::Sampler::Create_info sampler_create_info{
			.min_filter = gpu::Sampler::Filter::Nearest,
			.mag_filter = gpu::Sampler::Filter::Nearest,
			.mipmap_mode = gpu::Sampler::Mipmap_mode::Nearest,
			.max_lod = 0.0f
		};

		auto sampler = gpu::Sampler::create(device, sampler_create_info);
		if (!sampler) return sampler.error().forward("Create tonemapping sampler failed");

		auto fragment_shader = gpu::Graphic_shader::create(
			device,
			shader_asset::tonemapping_frag,
			gpu::Graphic_shader::Stage::Fragment,
			1,
			0,
			0,
			1
		);
		if (!fragment_shader)
			return fragment_shader.error().forward("Create tonemapping fragment shader failed");

		auto fullscreen_pass = graphics::Fullscreen_pass<true>::create(
			device,
			*fragment_shader,
			{.type = SDL_GPU_TEXTURETYPE_2D, .format = target_format, .usage = {.color_target = true}},
			{}
		);
		if (!fullscreen_pass)
			return fullscreen_pass.error().forward("Create tonemapping fullscreen pass failed");

		return Tonemapping(std::move(*fullscreen_pass), std::move(*sampler));
	}

	std::expected<void, util::Error> Tonemapping::render(
		const gpu::Command_buffer& command_buffer,
		SDL_GPUTexture* light_info_texture,
		SDL_GPUTexture* target_texture,
		const Param& param
	) noexcept
	{
		const auto light_info_binding =
			SDL_GPUTextureSamplerBinding{.texture = light_info_texture, .sampler = sampler};

		const auto sampler_texture_arr = std::array{
			light_info_binding,
		};

		command_buffer
			.push_uniform_to_fragment(0, util::as_bytes(glm::vec4(param.exposure, 0.0f, 0.0f, 0.0f)));

		return fullscreen_pass
			.render(command_buffer, target_texture, sampler_texture_arr, std::nullopt, std::nullopt)
			.transform_error(util::Error::forward_fn("Render tonemapping pass failed"));
	}
}