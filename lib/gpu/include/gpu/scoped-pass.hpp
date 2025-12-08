#pragma once

#include <SDL3/SDL_gpu.h>
#include <cassert>
#include <utility>

namespace gpu
{
	///
	/// @brief GPU Pass Template
	///
	template <typename T>
	class Scoped_pass
	{
	  protected:

		friend class Command_buffer;

		T* resource;

		Scoped_pass(T* resource) noexcept :
			resource(resource)
		{
			assert(resource != nullptr);
		}

		void delete_resource() noexcept;

	  public:

		Scoped_pass(const Scoped_pass&) = delete;
		Scoped_pass& operator=(const Scoped_pass&) = delete;

		Scoped_pass(Scoped_pass&& other) noexcept :
			resource(other.resource)
		{
			other.resource = nullptr;
		}

		Scoped_pass& operator=(Scoped_pass&& other) noexcept
		{
			if (&other != this)
			{
				this->~Scoped_pass();
				new (this) Scoped_pass(std::move(other));
			}

			return *this;
		}

		operator T*() const noexcept { return this->resource; }

		///
		/// @brief Ends the current pass
		///
		///
		void end() noexcept
		{
			assert(resource != nullptr);
			delete_resource();
			resource = nullptr;
		}

		~Scoped_pass() noexcept { assert(resource == nullptr); }
	};
}