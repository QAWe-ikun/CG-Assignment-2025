#pragma once

#include "buffer.hpp"
#include "compute-pipeline.hpp"
#include "scoped-pass.hpp"

#include <SDL3/SDL_gpu.h>
#include <span>

namespace gpu
{
	///
	/// @brief Compute pass
	///
	///
	class Compute_pass : public Scoped_pass<SDL_GPUComputePass>
	{
	  public:

		///
		/// @brief Bind a compute pipeline
		///
		/// @param pipeline Compute pipeline
		///
		void bind_pipeline(const Compute_pipeline& pipeline) const noexcept;

		///
		/// @brief Bind samplers
		///
		/// @param first_slot First binding slot
		/// @param samplers List of samplers
		///
		void bind_samplers(uint32_t first_slot, std::span<const SDL_GPUSampler> samplers) const noexcept;

		///
		/// @brief Bind storage textures
		///
		/// @param first_slot First binding slot
		/// @param textures List of textures
		///
		void bind_storage_textures(uint32_t first_slot, std::span<SDL_GPUTexture*> textures) const noexcept;

		///
		/// @brief Bind storage buffers
		///
		/// @param first_slot First binding slot
		/// @param buffers List of buffers
		///
		void bind_storage_buffers(uint32_t first_slot, std::span<const Buffer> buffers) const noexcept;

		///
		/// @brief Launch a compute task
		///
		/// @param group_count_x X-axis work group count
		/// @param group_count_y Y-axis work group count
		/// @param group_count_z Z-axis work group count
		///
		void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) const noexcept;

		///
		/// @brief Launch indirect compute task
		///
		/// @param buffer Indirect buffer
		/// @param offset Offset in the buffer in bytes
		///
		void dispatch_indirect(const Buffer& buffer, uint32_t offset) const noexcept;

	  private:

		using Scoped_pass<SDL_GPUComputePass>::Scoped_pass;
	};
}