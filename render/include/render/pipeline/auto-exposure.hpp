#pragma once

#include "gpu/command-buffer.hpp"
#include "gpu/compute-pipeline.hpp"
#include "gpu/sampler.hpp"
#include "gpu/texture.hpp"

#include "render/target/auto-exposure.hpp"
#include "render/target/light.hpp"

#include <expected>
#include <glm/glm.hpp>

namespace render::pipeline
{
	class AutoExposure
	{
	  public:

		struct Params
		{
			float min_luminance;
			float max_luminance;
			float eye_adaptation_rate;
			float delta_time;
		};

		static std::expected<AutoExposure, util::Error> create(SDL_GPUDevice* device) noexcept;

		std::expected<void, util::Error> compute(
			const gpu::CommandBuffer& command_buffer,
			const target::AutoExposure& target,
			const target::LightBuffer& light_buffer,
			const Params& params,
			glm::u32vec2 size
		) const noexcept;

	  private:

		struct HistogramParam
		{
			float min_histogram_value;
			float log_min_histogram_value;  // = log2(min)
			float dynamic_range;            // = log2(max) - log2(min)
			alignas(8) glm::uvec2 input_size;

			static HistogramParam from(const Params& params, glm::u32vec2 size) noexcept;
		};

		struct AvgParam
		{
			float min_histogram_value;
			float log_min_histogram_value;  // = log2(min)
			float dynamic_range;            // = log2(max) - log2(min)
			float adaptation_rate;

			static AvgParam from(const Params& params) noexcept;
		};

		gpu::ComputePipeline clear_pipeline;
		gpu::ComputePipeline histogram_pipeline;
		gpu::ComputePipeline avg_pipeline;
		gpu::Texture mask_texture;
		gpu::Sampler mask_sampler;

		AutoExposure(
			gpu::ComputePipeline clear_pipeline,
			gpu::ComputePipeline histogram_pipeline,
			gpu::ComputePipeline avg_pipeline,
			gpu::Texture mask_texture,
			gpu::Sampler mask_sampler
		) :
			clear_pipeline(std::move(clear_pipeline)),
			histogram_pipeline(std::move(histogram_pipeline)),
			avg_pipeline(std::move(avg_pipeline)),
			mask_texture(std::move(mask_texture)),
			mask_sampler(std::move(mask_sampler))
		{}

	  public:

		AutoExposure(const AutoExposure&) = delete;
		AutoExposure(AutoExposure&&) = default;
		AutoExposure& operator=(const AutoExposure&) = delete;
		AutoExposure& operator=(AutoExposure&&) = default;
	};
}