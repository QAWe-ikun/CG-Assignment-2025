#pragma once

#include <SDL3/SDL_gpu.h>
#include <gpu/buffer.hpp>
#include <map>
#include <memory>
#include <vector>

namespace graphics
{
	class Buffer_pool
	{
		struct Pool_key
		{
			uint32_t size;
			gpu::Buffer::Usage usage;

			auto operator<=>(const Pool_key&) const = default;
			bool operator==(const Pool_key&) const = default;
		};

		std::map<Pool_key, std::vector<std::shared_ptr<gpu::Buffer>>> backup_pool;
		std::vector<std::pair<Pool_key, std::shared_ptr<gpu::Buffer>>> in_use_buffers;

		SDL_GPUDevice* device;

	  public:

		Buffer_pool(SDL_GPUDevice* device) noexcept :
			device(device)
		{}

		///
		/// @brief Return all in-use buffers to the pool. Called before a new frame
		/// @note After calling this, all previously obtained buffer references should be treated as
		/// invalidated, as some of them may be cleaned up and reused
		/// @warning Not thread-safe
		///
		void cycle() noexcept;

		///
		/// @brief Acquire a buffer by size and usage
		/// @note Must be called after `cycle()`
		/// @warning Not thread-safe
		///
		/// @param usage Usage of the buffer
		/// @param size Size of the buffer in size
		/// @return Acquired buffer, or error if failed
		///
		std::expected<std::shared_ptr<gpu::Buffer>, util::Error> acquire_buffer(
			gpu::Buffer::Usage usage,
			uint32_t size
		) noexcept;

		///
		/// @brief Recycle unused buffers from the pool to free memory
		/// @note Should be called after all `acquire_buffer` in a frame
		/// @warning Not thread-safe
		///
		void gc() noexcept;

		Buffer_pool(const Buffer_pool&) = delete;
		Buffer_pool(Buffer_pool&&) = default;
		Buffer_pool& operator=(const Buffer_pool&) = delete;
		Buffer_pool& operator=(Buffer_pool&&) = default;
	};

	class Transfer_buffer_pool
	{
		struct Pool_key
		{
			gpu::Transfer_buffer::Usage usage;
			uint32_t size;

			auto operator<=>(const Pool_key&) const = default;
			bool operator==(const Pool_key&) const = default;
		};

		std::map<Pool_key, std::vector<std::shared_ptr<gpu::Transfer_buffer>>> backup_pool;
		std::vector<std::pair<Pool_key, std::shared_ptr<gpu::Transfer_buffer>>> in_use_buffers;

		SDL_GPUDevice* device;

	  public:

		Transfer_buffer_pool(SDL_GPUDevice* device) noexcept :
			device(device)
		{}

		///
		/// @brief Return all in-use buffers to the pool. Called before a new frame
		/// @note After calling this, all previously obtained buffer references should be treated as
		/// invalidated, as some of them may be cleaned up and reused
		/// @warning Not thread-safe
		///
		void cycle() noexcept;

		///
		/// @brief Acquire a transfer buffer by size and usage
		/// @note Must be called after `cycle()`
		/// @warning Not thread-safe
		///
		/// @param usage Usage of the buffer
		/// @param size Size of the buffer in size
		/// @return Acquired buffer, or error if failed
		///
		std::expected<std::shared_ptr<gpu::Transfer_buffer>, util::Error> acquire_buffer(
			gpu::Transfer_buffer::Usage usage,
			uint32_t size
		) noexcept;

		///
		/// @brief Recycle unused buffers from the pool to free memory
		/// @note Should be called after all `acquire_buffer` in a frame
		/// @warning Not thread-safe
		///
		void gc() noexcept;

		Transfer_buffer_pool(const Transfer_buffer_pool&) = delete;
		Transfer_buffer_pool(Transfer_buffer_pool&&) = default;
		Transfer_buffer_pool& operator=(const Transfer_buffer_pool&) = delete;
		Transfer_buffer_pool& operator=(Transfer_buffer_pool&&) = default;
	};
}