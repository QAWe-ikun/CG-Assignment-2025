#pragma once

#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"

#include <glm/glm.hpp>

namespace render::pipeline
{
	class DebugPipeline
	{
	  public:

		static std::expected<DebugPipeline, util::Error> create(
			SDL_GPUDevice* device,
			gpu::Texture::Format format
		) noexcept;

		void render_channels(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			SDL_GPUTexture* input_texture,
			glm::u32vec2 size,
			uint8_t channel_count
		) const noexcept;

	  private:

		graphics::FullscreenPass<false> FullscreenPass;
		gpu::Sampler sampler;

		DebugPipeline(graphics::FullscreenPass<false> FullscreenPass, gpu::Sampler sampler) noexcept :
			FullscreenPass(std::move(FullscreenPass)),
			sampler(std::move(sampler))
		{}

	  public:

		DebugPipeline(const DebugPipeline&) = delete;
		DebugPipeline(DebugPipeline&&) = default;
		DebugPipeline& operator=(const DebugPipeline&) = delete;
		DebugPipeline& operator=(DebugPipeline&&) = default;
	};
}