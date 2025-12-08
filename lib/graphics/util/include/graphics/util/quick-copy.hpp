#pragma once

#include "gpu/copy-pass.hpp"

#include <expected>
#include <functional>

namespace graphics
{
	// Create a command buffer and execute a copy task on it
	std::expected<void, util::Error> execute_copy_task(
		SDL_GPUDevice* device,
		const std::function<void(const gpu::Copy_pass&)>& task
	) noexcept;
}