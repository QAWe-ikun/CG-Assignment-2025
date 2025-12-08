#include "graphic/common/utility.hpp"

namespace graphic
{
	std::expected<void, util::Error> execute_copy_task(
		SDL_GPUDevice* device,
		const std::function<void(const gpu::Copy_pass&)>& task
	) noexcept
	{
		auto command_buffer = gpu::Command_buffer::acquire_from(device);
		if (!command_buffer) return command_buffer.error().propagate("Acquire command buffer failed");

		const auto copy_result = command_buffer->run_copy_pass(task);
		if (!copy_result) return copy_result.error().propagate("Run copy pass failed");

		const auto submit_result = command_buffer->submit();
		if (!submit_result) return submit_result.error().propagate("Submit command buffer failed");

		return {};
	}
}