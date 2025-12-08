#pragma once

#include "gpu/buffer.hpp"
#include "gpu/texture.hpp"
#include "image/repr.hpp"
#include "util/as-byte.hpp"

#include <expected>

namespace graphics
{
	namespace detail
	{
		struct Image_data
		{
			glm::u32vec2 size;
			std::span<const std::byte> pixels;
		};

		// Type-independent internal implementation of create_texture_from_image
		std::expected<gpu::Texture, util::Error> create_texture_from_image_internal(
			SDL_GPUDevice* device,
			gpu::Texture::Format format,
			Image_data image
		) noexcept;

		// Type-independent internal implementation of create_texture_from_mipmap
		std::expected<gpu::Texture, util::Error> create_texture_from_mipmap_internal(
			SDL_GPUDevice* device,
			gpu::Texture::Format format,
			std::span<const Image_data> mipmap_chain
		) noexcept;
	}

	///
	/// @brief Create a buffer and uploads input binary data to it. No cycling is performed.
	/// @details
	/// - This function is designed for initializing buffers with data at **loading stage**.
	/// - It has some overhead, which should be acceptable at loading stage but not at render-time.
	/// - Don't use it on-the-fly during rendering. Manually copy on a copy pass from the main command buffer.
	///
	/// @param usage Buffer usage
	/// @param data Binary data
	/// @return Created buffer, or error
	///
	std::expected<gpu::Buffer, util::Error> create_buffer_from_data(
		SDL_GPUDevice* device,
		gpu::Buffer::Usage usage,
		std::span<const std::byte> data
	) noexcept;

	///
	/// @brief Create a texture from image data
	/// @details
	/// - This function is designed for initializing textures with data at **loading stage**.
	/// - It has some overhead, which should be acceptable at loading stage but not at render-time.
	/// - Don't use it on-the-fly during rendering. Manually copy on a copy pass from the main command buffer.
	///
	/// @tparam T Image pixel type
	/// @param format Image format
	/// @param image Image object
	/// @return Created texture, or error
	///
	template <typename T>
	std::expected<gpu::Texture, util::Error> create_texture_from_image(
		SDL_GPUDevice* device,
		gpu::Texture::Format format,
		const image::Image_container<T>& image
	) noexcept
	{
		return detail::create_texture_from_image_internal(
			device,
			format,
			{.size = image.size, .pixels = util::as_bytes(image.pixels)}
		);
	}

	///
	/// @brief Create a texture from mipmap chain
	/// @details
	/// - This function is designed for initializing textures with data at **loading stage**.
	/// - It has some overhead, which should be acceptable at loading stage but not at render-time.
	/// - Don't use it on-the-fly during rendering. Manually copy on a copy pass from the main command buffer.
	///
	/// @tparam T Image pixel type
	/// @param format Image format
	/// @param mipmap_chain Image mipmap chain
	/// @return Created texture, or error
	///
	template <typename T>
	std::expected<gpu::Texture, util::Error> create_texture_from_mipmap(
		SDL_GPUDevice* device,
		gpu::Texture::Format format,
		const std::vector<image::Image_container<T>>& mipmap_chain
	) noexcept
	{
		std::vector<detail::Image_data> chain_data;
		chain_data.reserve(mipmap_chain.size());
		for (const auto& level : mipmap_chain)
			chain_data.push_back(
				detail::Image_data{.size = level.size, .pixels = util::as_bytes(level.pixels)}
			);

		return detail::create_texture_from_mipmap_internal(device, format, chain_data);
	}
}