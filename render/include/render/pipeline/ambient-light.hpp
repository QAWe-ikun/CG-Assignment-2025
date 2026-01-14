#pragma once

#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"

#include "render/target/ao.hpp"
#include "render/target/gbuffer.hpp"

#include <glm/glm.hpp>

namespace render::pipeline
{
	class AmbientLight
	{
	  public:

		struct Param
		{
			glm::vec3 ambient_intensity;  // in reference luminance
			float ao_strength;
		};

		static std::expected<AmbientLight, util::Error> create(SDL_GPUDevice* device) noexcept;

		void render(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const target::Gbuffer& gbuffer,
			const target::AO& ao,
			const Param& param
		) const noexcept;

	  private:

		graphics::FullscreenPass<false> FullscreenPass;
		gpu::Sampler sampler_nearest;
		gpu::Sampler sampler_linear;

		AmbientLight(
			graphics::FullscreenPass<false> FullscreenPass,
			gpu::Sampler sampler_nearest,
			gpu::Sampler sampler_linear
		) noexcept :
			FullscreenPass(std::move(FullscreenPass)),
			sampler_nearest(std::move(sampler_nearest)),
			sampler_linear(std::move(sampler_linear))
		{}

	  public:

		AmbientLight(const AmbientLight&) = delete;
		AmbientLight(AmbientLight&&) = default;
		AmbientLight& operator=(const AmbientLight&) = delete;
		AmbientLight& operator=(AmbientLight&&) = default;
	};
}