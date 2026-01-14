#pragma once

#include "gpu/sampler.hpp"
#include "graphics/util/fullscreen-pass.hpp"

#include <glm/glm.hpp>

namespace render::pipeline
{
	///
	/// @brief Preetham sky model
	/// @note Algorithm adapted from https://github.com/diharaw/SkyModels
	///
	class SkyPreetham
	{
	  public:

		struct Params
		{
			glm::mat4 camera_mat_inv;
			glm::uvec2 screen_size;
			glm::vec3 eye_position;
			glm::vec3 sun_direction;
			glm::vec3 sun_intensity;
			float turbidity;
		};

		static std::expected<SkyPreetham, util::Error> create(SDL_GPUDevice* device) noexcept;

		void render(
			const gpu::CommandBuffer& command_buffer,
			const gpu::RenderPass& render_pass,
			const Params& params
		) const noexcept;

	  private:

		struct alignas(16) PreethamParam
		{
			alignas(16) glm::vec3 A;
			alignas(16) glm::vec3 B;
			alignas(16) glm::vec3 C;
			alignas(16) glm::vec3 D;
			alignas(16) glm::vec3 E;
			alignas(16) glm::vec3 Z;

			PreethamParam(float turbidity, glm::vec3 sun_direction) noexcept;
		};

		struct InternalParam
		{
			glm::mat4 camera_mat_inv;
			glm::vec2 screen_size;
			alignas(16) glm::vec3 eye_position;
			alignas(16) glm::vec3 sun_direction;
			alignas(16) glm::vec3 sun_intensity;
			float intensity;

			PreethamParam preetham;

			InternalParam(const Params& params) noexcept;
		};

		gpu::Sampler sampler;
		graphics::FullscreenPass<false> FullscreenPass;

		SkyPreetham(gpu::Sampler sampler, graphics::FullscreenPass<false> FullscreenPass) noexcept :
			sampler(std::move(sampler)),
			FullscreenPass(std::move(FullscreenPass))
		{}

	  public:

		SkyPreetham(const SkyPreetham&) = delete;
		SkyPreetham(SkyPreetham&&) = default;
		SkyPreetham& operator=(const SkyPreetham&) = delete;
		SkyPreetham& operator=(SkyPreetham&&) = default;
	};
}