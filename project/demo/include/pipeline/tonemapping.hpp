#pragma once

#include "gpu/command-buffer.hpp"
#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"

#include <glm/glm.hpp>

namespace pipeline
{
	class Tonemapping
	{
	  public:

		struct Param
		{
			float exposure = 1.0f;
		};

	  private:

		graphics::Fullscreen_pass<true> fullscreen_pass;
		gpu::Sampler sampler;

		Tonemapping(graphics::Fullscreen_pass<true> fullscreen_pass, gpu::Sampler sampler) :
			fullscreen_pass(std::move(fullscreen_pass)),
			sampler(std::move(sampler))
		{}

	  public:

		static std::expected<Tonemapping, util::Error> create(
			SDL_GPUDevice* device,
			SDL_GPUTextureFormat target_format
		) noexcept;

		std::expected<void, util::Error> render(
			const gpu::Command_buffer& command_buffer,
			SDL_GPUTexture* light_info_texture,
			SDL_GPUTexture* target_texture,
			const Param& param
		) noexcept;

		Tonemapping(const Tonemapping&) = delete;
		Tonemapping(Tonemapping&&) = default;
		Tonemapping& operator=(const Tonemapping&) = delete;
		Tonemapping& operator=(Tonemapping&&) = default;
	};
}