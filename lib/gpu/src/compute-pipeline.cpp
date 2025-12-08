#include "gpu/compute-pipeline.hpp"
#include "gpu/util.hpp"

namespace gpu
{
	std::expected<Compute_pipeline, util::Error> Compute_pipeline::create(
		SDL_GPUDevice* device,
		std::span<const std::byte> shader_data,
		uint32_t num_samplers,
		uint32_t num_readonly_storage_textures,
		uint32_t num_readwrite_storage_textures,
		uint32_t num_readonly_storage_buffers,
		uint32_t num_readwrite_storage_buffers,
		uint32_t num_uniform_buffers,
		uint32_t threadcount_x,
		uint32_t threadcount_y,
		uint32_t threadcount_z
	) noexcept
	{
		assert(device != nullptr);
		assert(shader_data.data() != nullptr);
		assert(!shader_data.empty());

		const SDL_GPUComputePipelineCreateInfo create_info{
			.code_size = shader_data.size(),
			.code = reinterpret_cast<const uint8_t*>(shader_data.data()),
			.entrypoint = "main",
			.format = SDL_GPU_SHADERFORMAT_SPIRV,
			.num_samplers = num_samplers,
			.num_readonly_storage_textures = num_readonly_storage_textures,
			.num_readonly_storage_buffers = num_readonly_storage_buffers,
			.num_readwrite_storage_textures = num_readwrite_storage_textures,
			.num_readwrite_storage_buffers = num_readwrite_storage_buffers,
			.num_uniform_buffers = num_uniform_buffers,
			.threadcount_x = threadcount_x,
			.threadcount_y = threadcount_y,
			.threadcount_z = threadcount_z,
			.props = 0
		};

		auto* const pipeline = SDL_CreateGPUComputePipeline(device, &create_info);
		if (pipeline == nullptr) RETURN_SDL_ERROR;

		return Compute_pipeline(device, pipeline);
	}
}