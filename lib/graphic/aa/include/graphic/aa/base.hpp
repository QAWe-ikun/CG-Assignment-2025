#pragma once

#include <glm/glm.hpp>
#include <gpu.hpp>

namespace graphic::aa
{
	///
	/// @brief Antialiasing Processor Interface
	///
	///
	class Processor
	{
	  public:

		virtual ~Processor() noexcept = default;

		///
		/// @brief Run antialising processor
		///
		/// @param source Input source texture, should have a size of `size`
		/// @param target Target texture, should have a size of `size
		/// @param size Size of the textures, in pixels
		/// @return `void` on success, or `util::Error` on failure
		///
		virtual std::expected<void, util::Error> run_antialiasing(
			SDL_GPUDevice* device,
			const gpu::Command_buffer& command_buffer,
			SDL_GPUTexture* source,
			SDL_GPUTexture* target,
			glm::u32vec2 size
		) noexcept = 0;
	};

}