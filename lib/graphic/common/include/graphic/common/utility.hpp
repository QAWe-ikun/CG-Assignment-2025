#pragma once

#include <gpu.hpp>

namespace graphic
{
	///
	/// @brief Creates a command buffer, executes the copy task then submits it
	///
	/// @param task Copy task
	///
	std::expected<void, util::Error> execute_copy_task(
		SDL_GPUDevice* device,
		const std::function<void(const gpu::Copy_pass&)>& task
	) noexcept;
}