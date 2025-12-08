#pragma once

#include <expected>
#include <gpu/sampler.hpp>
#include <optional>
#include <tiny_gltf.h>

namespace gltf
{
	// Sampler creation configuration
	struct Sampler_config
	{
		float lod_bias = 0.0f;                   // LOD bias
		std::optional<float> anisotropy = 4.0f;  // Max anisotropy, nullopt to disable anisotropic filtering
	};

	///
	/// @brief Create a GPU sampler from a tinygltf sampler
	///
	/// @param sampler Tinygltf sampler
	/// @param config Additional sampler configuration
	/// @return GPU sampler on success, or error on failure
	///
	std::expected<gpu::Sampler, util::Error> create_sampler(
		SDL_GPUDevice* device,
		const tinygltf::Sampler& sampler,
		const Sampler_config& config
	) noexcept;
}