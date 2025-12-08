#pragma once

#include <SDL3/SDL_gpu.h>
#include <cassert>
#include <util/error.hpp>
#include <utility>

namespace gpu
{
	///
	/// @brief Smart wrapper for SDL-GPU resources
	///
	/// @tparam T Resource Type
	///
	template <typename T>
	class Resource_box
	{
	  protected:

		SDL_GPUDevice* device;
		T* resource;

		// Resource deleter
		void delete_resource() noexcept;

	  public:

		Resource_box(const Resource_box&) = delete;
		Resource_box& operator=(const Resource_box&) = delete;

		Resource_box(Resource_box&& other) noexcept :
			device(other.device),
			resource(other.resource)
		{
			other.device = nullptr;
			other.resource = nullptr;
		}

		Resource_box& operator=(Resource_box&& other) noexcept
		{
			if (&other != this)
			{
				this->~Resource_box();
				new (this) Resource_box(std::move(other));
			}

			return *this;
		}

		Resource_box(SDL_GPUDevice* device, T* resource) noexcept :
			device(device),
			resource(resource)
		{
			assert(device != nullptr);
			assert(resource != nullptr);
		}

		~Resource_box() noexcept
		{
			if (device == nullptr || resource == nullptr) return;
			delete_resource();
		}

		operator T*() const noexcept { return this->resource; }
	};
}
