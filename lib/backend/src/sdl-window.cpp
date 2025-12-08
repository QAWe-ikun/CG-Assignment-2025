#include "backend/sdl-window.hpp"

#include <SDL3/SDL_init.h>
#include <format>

namespace backend
{
	std::expected<void, util::Error> initialize_sdl()
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
			return util::Error(std::format("初始化 SDL 失败: {}", SDL_GetError()));

		return {};
	}

	void shutdown_sdl() noexcept
	{
		SDL_Quit();
	}

	static SDL_Window* sdl_window = nullptr;
	static SDL_GPUDevice* sdl_gpu_device = nullptr;

	std::expected<void, util::Error> create_window_and_device(
		int width,
		int height,
		const char* title,
		bool debug_enabled
	)
	{
		SDL_Window* const window =
			SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

		if (window == nullptr) return util::Error(std::format("创建窗口失败: {}", SDL_GetError()));

		SDL_GPUDevice* const gpu_device =
			SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, debug_enabled, "vulkan");
		if (gpu_device == nullptr)
		{
			SDL_DestroyWindow(window);
			return util::Error(std::format("创建 GPU 设备失败: {}", SDL_GetError()));
		}

		if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
		{
			SDL_DestroyGPUDevice(gpu_device);
			SDL_DestroyWindow(window);
			return util::Error(std::format("将窗口与 GPU 设备关联失败: {}", SDL_GetError()));
		}

		SDL_SetGPUSwapchainParameters(
			gpu_device,
			window,
			SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
			SDL_GPU_PRESENTMODE_VSYNC
		);

		sdl_window = window;
		sdl_gpu_device = gpu_device;

		return {};
	}

	SDL_Window* get_sdl_window() noexcept
	{
		return sdl_window;
	}

	SDL_GPUDevice* get_sdl_gpu_device() noexcept
	{
		return sdl_gpu_device;
	}

	float get_window_scale() noexcept
	{
		return SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	}

	glm::ivec2 get_window_size() noexcept
	{
		glm::ivec2 size;
		SDL_GetWindowSize(sdl_window, &size.x, &size.y);
		return size;
	}

	SDL_GPUTextureFormat get_swapchain_texture_format() noexcept
	{
		return SDL_GetGPUSwapchainTextureFormat(sdl_gpu_device, sdl_window);
	}
}