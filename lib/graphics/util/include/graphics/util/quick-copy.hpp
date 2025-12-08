#pragma once

#include <gpu.hpp>

namespace graphics
{
	// Create a command buffer and execute a copy task on it
	std::expected<void, util::Error> execute_copy_task(
		SDL_GPUDevice* device,
		const std::function<void(const gpu::Copy_pass&)>& task
	) noexcept;
}