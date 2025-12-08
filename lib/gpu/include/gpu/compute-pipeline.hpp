#pragma once

#include <SDL3/SDL_gpu.h>
#include <expected>
#include <span>

#include "resource-box.hpp"
#include "util/error.hpp"

namespace gpu
{
	///
	/// @brief Compute pipeline
	///
	///
	class Compute_pipeline : public Resource_box<SDL_GPUComputePipeline>
	{
	  public:

		Compute_pipeline(const Compute_pipeline&) = delete;
		Compute_pipeline& operator=(const Compute_pipeline&) = delete;
		Compute_pipeline(Compute_pipeline&&) noexcept = default;
		Compute_pipeline& operator=(Compute_pipeline&&) noexcept = default;
		~Compute_pipeline() noexcept = default;

		struct Create_info
		{
			std::span<const std::byte> shader_data;
			uint32_t num_samplers = 0;
			uint32_t num_readonly_storage_textures = 0;
			uint32_t num_readwrite_storage_textures = 0;
			uint32_t num_readonly_storage_buffers = 0;
			uint32_t num_readwrite_storage_buffers = 0;
			uint32_t num_uniform_buffers = 0;
			uint32_t threadcount_x;
			uint32_t threadcount_y;
			uint32_t threadcount_z;
		};

		///
		/// @brief Create a compute pipeline
		///
		/// @return Compute pipeline object, or error if failed
		///
		static std::expected<Compute_pipeline, util::Error> create(
			SDL_GPUDevice* device,
			const Create_info& create_info,
			const std::string& name
		) noexcept;

	  private:

		using Resource_box<SDL_GPUComputePipeline>::Resource_box;
	};
}