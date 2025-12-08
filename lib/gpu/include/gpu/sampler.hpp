#pragma once

#include <SDL3/SDL_gpu.h>
#include <expected>

#include "resource-box.hpp"

namespace gpu
{
	class Sampler : public Resource_box<SDL_GPUSampler>
	{
	  public:

		Sampler(const Sampler&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		Sampler(Sampler&&) = default;
		Sampler& operator=(Sampler&&) = default;
		~Sampler() noexcept = default;

		///
		/// @brief Creates a sampler object
		///
		/// @param create_info Sampler create info
		/// @return Sampler object, or error if failed
		///
		static std::expected<Sampler, util::Error> create(
			SDL_GPUDevice* device,
			const SDL_GPUSamplerCreateInfo& create_info
		) noexcept;

	  private:

		using Resource_box<SDL_GPUSampler>::Resource_box;
	};
}