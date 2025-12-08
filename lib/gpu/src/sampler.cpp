#include "gpu/sampler.hpp"
#include "gpu/util.hpp"

namespace gpu
{
	std::expected<Sampler, util::Error> Sampler::create(
		SDL_GPUDevice* device,
		const SDL_GPUSamplerCreateInfo& create_info
	) noexcept
	{
		assert(device != nullptr);

		auto* const sampler = SDL_CreateGPUSampler(device, &create_info);
		if (sampler == nullptr) RETURN_SDL_ERROR;
		return Sampler(device, sampler);
	}
}