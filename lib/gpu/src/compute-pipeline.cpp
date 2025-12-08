#include "gpu/compute-pipeline.hpp"
#include "gpu/util.hpp"

namespace gpu
{
	std::expected<Compute_pipeline, util::Error> Compute_pipeline::create(
		SDL_GPUDevice* device,
		const Create_info& create_info
	) noexcept
	{
		assert(device != nullptr);
		assert(create_info.shader_data.data() != nullptr);
		assert(!create_info.shader_data.empty());

		const SDL_GPUComputePipelineCreateInfo sdl_create_info{
			.code_size = create_info.shader_data.size(),
			.code = reinterpret_cast<const uint8_t*>(create_info.shader_data.data()),
			.entrypoint = "main",
			.format = SDL_GPU_SHADERFORMAT_SPIRV,
			.num_samplers = create_info.num_samplers,
			.num_readonly_storage_textures = create_info.num_readonly_storage_textures,
			.num_readonly_storage_buffers = create_info.num_readonly_storage_buffers,
			.num_readwrite_storage_textures = create_info.num_readwrite_storage_textures,
			.num_readwrite_storage_buffers = create_info.num_readwrite_storage_buffers,
			.num_uniform_buffers = create_info.num_uniform_buffers,
			.threadcount_x = create_info.threadcount_x,
			.threadcount_y = create_info.threadcount_y,
			.threadcount_z = create_info.threadcount_z,
			.props = 0
		};

		// https://github.com/libsdl-org/SDL/blob/aae2f74ae611652b2858393fdf6e4d6d6cb85384/src/gpu/vulkan/SDL_gpu_vulkan.c#L3780

		auto* const pipeline = SDL_CreateGPUComputePipeline(device, &sdl_create_info);
		if (pipeline == nullptr) RETURN_SDL_ERROR;

		return Compute_pipeline(device, pipeline);
	}
}