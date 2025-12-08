#include "gpu/compute-pass.hpp"
#include <SDL3/SDL_gpu.h>

namespace gpu
{
	void Compute_pass::bind_pipeline(const Compute_pipeline& pipeline) const noexcept
	{
		assert(resource != nullptr);
		SDL_BindGPUComputePipeline(resource, pipeline);
	}

	void Compute_pass::bind_samplers(
		uint32_t first_slot,
		std::span<const SDL_GPUTextureSamplerBinding> samplers
	) const noexcept
	{
		assert(resource != nullptr);
		SDL_BindGPUComputeSamplers(
			resource,
			first_slot,
			samplers.data(),
			static_cast<uint32_t>(samplers.size())
		);
	}

	void Compute_pass::bind_storage_textures(
		uint32_t first_slot,
		std::span<SDL_GPUTexture* const> textures
	) const noexcept
	{
		assert(resource != nullptr);
		SDL_BindGPUComputeStorageTextures(
			resource,
			first_slot,
			textures.data(),
			static_cast<uint32_t>(textures.size())
		);
	}

	void Compute_pass::bind_storage_buffers(
		uint32_t first_slot,
		std::span<SDL_GPUBuffer* const> buffers
	) const noexcept
	{
		assert(resource != nullptr);
		SDL_BindGPUComputeStorageBuffers(
			resource,
			first_slot,
			buffers.data(),
			static_cast<uint32_t>(buffers.size())
		);
	}

	void Compute_pass::dispatch(
		uint32_t group_count_x,
		uint32_t group_count_y,
		uint32_t group_count_z
	) const noexcept
	{
		assert(resource != nullptr);
		SDL_DispatchGPUCompute(resource, group_count_x, group_count_y, group_count_z);
	}

	void Compute_pass::dispatch_indirect(const Buffer& buffer, uint32_t offset) const noexcept
	{
		assert(resource != nullptr);
		SDL_DispatchGPUComputeIndirect(resource, buffer, offset);
	}
}