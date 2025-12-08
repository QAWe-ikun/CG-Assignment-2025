#pragma once

#include "gpu/texture.hpp"

#include "util/error.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <queue>

namespace graphics
{
	///
	/// @brief Smart texture, provides resize functionality
	/// @details #### Usage:
	/// - Assigns a format at construction
	/// - Resize using `resize` when needed, eg. after acquiring the swapchain
	/// - Use `operator*` to get the underlying SDL_GPUTexture pointer, valid until the next resize
	/// @note 2D, single-layer mipmap textures only
	///
	class Auto_texture
	{
	  public:

		Auto_texture(const Auto_texture&) = delete;
		Auto_texture& operator=(const Auto_texture&) = delete;
		Auto_texture(Auto_texture&&) = default;
		Auto_texture& operator=(Auto_texture&&) = default;

		Auto_texture(gpu::Texture::Format format) noexcept;
		~Auto_texture() = default;

		///
		/// @brief Resize the texture
		/// @note This invalidates any previously obtained texture pointers
		///
		/// @param size New size
		///
		std::expected<void, util::Error> resize(SDL_GPUDevice* device, glm::u32vec2 size) noexcept;

		///
		/// @brief Get the size of the texture
		///
		/// @return Current size
		///
		glm::u32vec2 get_size() const noexcept;

		///
		/// @brief Get the texture pointer
		/// @note
		/// - The acquired pointer will be invalidated upon the next resize
		/// - `resize` must be called at least once before getting the pointer
		///
		SDL_GPUTexture* operator*() const noexcept;

	  private:

		glm::u32vec2 size = {0, 0};
		gpu::Texture::Format format;
		std::unique_ptr<gpu::Texture> texture;
	};

	///
	/// @brief Smart texture that cycles between multiple textures each frame, to store previous frame data
	/// @note Follows similar API to @p Smart_texture
	///
	class Cycle_texture
	{
	  public:

		Cycle_texture(const Cycle_texture&) = delete;
		Cycle_texture(Cycle_texture&&) noexcept = default;
		Cycle_texture& operator=(const Cycle_texture&) = delete;
		Cycle_texture& operator=(Cycle_texture&&) = default;

		///
		/// @brief Construct a Cycle_texture
		///
		/// @param format Format of the texture
		/// @param extra_pool_size Extra pool size, besides current and previous frame textures
		///
		Cycle_texture(gpu::Texture::Format format, size_t extra_pool_size = 1) noexcept;

		///
		/// @brief Resize the texture, and advance to the next texture in the cycle
		/// @note This invalidates any previously obtained texture pointers
		///
		/// @param size New size
		///
		std::expected<void, util::Error> resize_and_cycle(
			SDL_GPUDevice* device,
			glm::u32vec2 new_size
		) noexcept;

		///
		/// @brief Get current size of the textures
		///
		/// @return Current size
		///
		glm::u32vec2 get_size() const noexcept;

		///
		/// @brief Get the current frame texture
		///
		/// @return Current frame texture pointer
		///
		SDL_GPUTexture* current() const noexcept;

		///
		/// @brief Get the previous frame texture
		///
		/// @return Previous frame texture pointer
		///
		SDL_GPUTexture* prev() const noexcept;

	  private:

		glm::u32vec2 size = {0, 0};
		gpu::Texture::Format format;
		size_t extra_pool_size = 1;

		std::vector<gpu::Texture> texture_pool;
	};
}