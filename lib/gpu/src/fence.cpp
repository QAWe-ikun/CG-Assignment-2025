#include "gpu/fence.hpp"
#include "gpu/util.hpp"

namespace gpu
{
	bool Fence::is_signaled() const noexcept
	{
		assert(resource != nullptr);
		return SDL_QueryGPUFence(device, resource);
	}

	std::expected<void, util::Error> Fence::wait() const noexcept
	{
		assert(resource != nullptr);
		if (!SDL_WaitForGPUFences(device, false, &resource, 1)) RETURN_SDL_ERROR;
		return {};
	}

	std::expected<void, util::Error> Fence::wait_any(
		SDL_GPUDevice* device,
		std::span<SDL_GPUFence* const> fences
	) noexcept
	{
		assert(device != nullptr);
		assert(fences.data() != nullptr);
		assert(!fences.empty());

		if (!SDL_WaitForGPUFences(device, false, fences.data(), static_cast<uint32_t>(fences.size())))
			RETURN_SDL_ERROR;

		return {};
	}

	std::expected<void, util::Error> Fence::wait_all(
		SDL_GPUDevice* device,
		std::span<SDL_GPUFence* const> fences
	) noexcept
	{
		assert(device != nullptr);
		assert(fences.data() != nullptr);
		assert(!fences.empty());

		if (!SDL_WaitForGPUFences(device, true, fences.data(), static_cast<uint32_t>(fences.size())))
			RETURN_SDL_ERROR;

		return {};
	}

}