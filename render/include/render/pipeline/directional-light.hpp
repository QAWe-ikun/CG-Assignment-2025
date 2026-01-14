#pragma once

#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"
#include "render/target/gbuffer.hpp"
#include "render/target/shadow.hpp"

#include <glm/glm.hpp>

namespace render::pipeline
{
	class Directional_light
	{
	  public:

		struct Params
		{
			glm::mat4 camera_matrix_inv;
			glm::mat4 shadow_matrix_level0;
			glm::mat4 shadow_matrix_level1;
			glm::mat4 shadow_matrix_level2;
			alignas(16) glm::vec3 eye_position;
			alignas(16) glm::vec3 light_direction;
			alignas(16) glm::vec3 light_color;  // in reference luminance
		};

		static std::expected<Directional_light, util::Error> create(SDL_GPUDevice* device) noexcept;

		void render(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const target::Gbuffer& gbuffer,
			const target::Shadow& shadow,
			const Params& params
		) const noexcept;

	  private:

		graphics::FullscreenPass<false> FullscreenPass;
		gpu::Sampler sampler;
		gpu::Sampler shadow_sampler;

		Directional_light(
			graphics::FullscreenPass<false> FullscreenPass,
			gpu::Sampler sampler,
			gpu::Sampler shadow_sampler
		) noexcept :
			FullscreenPass(std::move(FullscreenPass)),
			sampler(std::move(sampler)),
			shadow_sampler(std::move(shadow_sampler))
		{}

	  public:

		Directional_light(const Directional_light&) = delete;
		Directional_light(Directional_light&&) = default;
		Directional_light& operator=(const Directional_light&) = delete;
		Directional_light& operator=(Directional_light&&) = default;
	};
}