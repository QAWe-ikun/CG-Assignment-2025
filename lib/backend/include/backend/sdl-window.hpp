#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <expected>
#include <glm/glm.hpp>
#include <util/error.hpp>

namespace backend
{
	///
	/// @brief Initialize SDL Library
	///
	std::expected<void, util::Error> initialize_sdl();

	///
	/// @brief Shut down SDL Library
	///
	///
	void shutdown_sdl() noexcept;

	///
	/// @brief Create SDL Window and GPU Device
	///
	/// @param width Initial width of the window
	/// @param height Initial height of the window
	/// @param title Initial title of the window
	///
	std::expected<void, util::Error> create_window_and_device(
		int width,
		int height,
		const char* title,
		bool debug_enabled = false
	);

	///
	/// @brief Get SDL_Window pointer
	///
	SDL_Window* get_sdl_window() noexcept;

	///
	/// @brief Get SDL_GPUDevice pointer
	///
	SDL_GPUDevice* get_sdl_gpu_device() noexcept;

	///
	/// @brief Get the scaling factor of the window
	///
	float get_window_scale() noexcept;

	///
	/// @brief Get the size of the window in pixels
	///
	glm::ivec2 get_window_size() noexcept;

	///
	/// @brief Get swapchain format of the window
	///
	SDL_GPUTextureFormat get_swapchain_texture_format() noexcept;
}