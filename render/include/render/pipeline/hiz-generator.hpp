#pragma once

#include "gpu/command-buffer.hpp"
#include "gpu/compute-pipeline.hpp"
#include "render/target/gbuffer.hpp"
#include "util/error.hpp"

#include <glm/fwd.hpp>

namespace render::pipeline
{
	class HizGenerator
	{
	  public:

		static std::expected<HizGenerator, util::Error> create(SDL_GPUDevice* device) noexcept;

		std::expected<void, util::Error> generate(
			const gpu::CommandBuffer& command_buffer,
			const target::Gbuffer& gbuffer,
			glm::u32vec2 hiz_top_size
		) const noexcept;

	  private:

		struct Param
		{
			glm::i32vec2 src_size;
			glm::i32vec2 dst_size;
		};

		gpu::ComputePipeline pipeline;

		HizGenerator(gpu::ComputePipeline pipeline) :
			pipeline(std::move(pipeline))
		{}

	  public:

		HizGenerator(const HizGenerator&) = delete;
		HizGenerator(HizGenerator&&) = default;
		HizGenerator& operator=(const HizGenerator&) = delete;
		HizGenerator& operator=(HizGenerator&&) = default;
	};
}