#pragma once

#include "gpu/command-buffer.hpp"
#include "gpu/compute-pipeline.hpp"
#include "gpu/sampler.hpp"

#include "render/target/auto-exposure.hpp"
#include "render/target/bloom.hpp"
#include "render/target/light.hpp"

namespace render::pipeline
{
	class Bloom
	{
	  public:

		struct Param
		{
			float start_threshold;
			float end_threshold;
			float attenuation;
		};

		static std::expected<Bloom, util::Error> create(SDL_GPUDevice* device) noexcept;

		std::expected<void, util::Error> render(
			const gpu::CommandBuffer& command_buffer,
			const target::LightBuffer& light_buffer_target,
			const target::Bloom& bloom_target,
			const target::AutoExposure& auto_exposure_target,
			const Param& param,
			glm::u32vec2 swapchain_size
		) const noexcept;

	  private:

		struct FilterParam
		{
			float start_threshold;
			float end_threshold;

			FilterParam(const Param& param) :
				start_threshold(param.start_threshold),
				end_threshold(param.end_threshold)
			{}
		};

		struct AddParam
		{
			float attenuation;

			AddParam(const Param& param) :
				attenuation(param.attenuation)
			{}
		};

		gpu::ComputePipeline filter;
		gpu::ComputePipeline blur;
		gpu::ComputePipeline blur_and_add;
		gpu::Sampler linear_sampler;

		Bloom(
			gpu::ComputePipeline filter,
			gpu::ComputePipeline blur,
			gpu::ComputePipeline blur_and_add,
			gpu::Sampler linear_sampler
		) :
			filter(std::move(filter)),
			blur(std::move(blur)),
			blur_and_add(std::move(blur_and_add)),
			linear_sampler(std::move(linear_sampler))
		{}

		std::expected<void, util::Error> run_filter_pass(
			const gpu::CommandBuffer& command_buffer,
			SDL_GPUTexture* input_texture,
			SDL_GPUTexture* output_texture,
			SDL_GPUBuffer* exposure_buffer,
			const Param& param,
			glm::u32vec2 swapchain_size
		) const noexcept;

		std::expected<void, util::Error> run_blur_pass(
			const gpu::CommandBuffer& command_buffer,
			const target::Bloom& bloom_target,
			uint32_t upsample_mip_level,
			glm::u32vec2 swapchain_size
		) const noexcept;

		void run_blit(
			const gpu::CommandBuffer& command_buffer,
			const target::Bloom& bloom_target,
			uint32_t upsample_mip_level,
			glm::u32vec2 swapchain_size
		) const noexcept;

		void run_initial_blit(
			const gpu::CommandBuffer& command_buffer,
			const target::Bloom& bloom_target,
			glm::u32vec2 swapchain_size
		) const noexcept;

		std::expected<void, util::Error> run_add_pass(
			const gpu::CommandBuffer& command_buffer,
			const target::Bloom& bloom_target,
			uint32_t upsample_mip_level,
			const Param& param,
			glm::u32vec2 swapchain_size
		) const noexcept;

	  public:

		Bloom(const Bloom&) = delete;
		Bloom(Bloom&&) = default;
		Bloom& operator=(const Bloom&) = delete;
		Bloom& operator=(Bloom&&) = default;
	};
}