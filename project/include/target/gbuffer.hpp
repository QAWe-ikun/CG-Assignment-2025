#pragma once

#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>

#include "graphics/util/smart-texture.hpp"

namespace target
{
	///
	/// @brief Gbuffer Render Target
	///
	/// @details
	/// #### Depth Texture
	/// Format: `D32@FLOAT + S@UINT8`
	/// - `D32`: 32-bit Floating Point Depth
	/// - `S8`: 8-bit Stencil, for multiple light sources
	///
	/// #### Albedo Texture
	/// Format: `RGBA@UNORM8_SRGB`
	/// - `RGB`: Albedo Color
	/// - `A`: Temporary coverage value (`0.0`=None, `1.0`=Fully covered), may later be replace with stencil
	///
	/// #### Lighting Info Texture
	/// Format: `RG@UINT32`
	/// - `R[15:0], R[31:16]`: Octahedral-encoded Normal
	/// - `G[7:0]`: Metallic
	/// - `G[15:8]`: Roughness
	/// - `G[23:16]`: None
	/// - `G[31:24]`: AO
	///
	struct Gbuffer
	{
		/* Texture Formats */

		// Depth Texture Format
		static constexpr gpu::Texture::Format depth_format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,
			.usage = {.sampler = true, .depth_stencil_target = true}
		};

		// Depth Value Format
		static constexpr gpu::Texture::Format depth_value_format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
			.usage = {.sampler = true, .color_target = true}
		};

		// Albedo Texture Format
		static constexpr gpu::Texture::Format albedo_format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
			.usage = {.sampler = true, .color_target = true}
		};

		// Lighting Info Texture Format
		static constexpr gpu::Texture::Format lighting_info_format{
			.type = SDL_GPU_TEXTURETYPE_2D,
			.format = SDL_GPU_TEXTUREFORMAT_R32G32_UINT,
			.usage = {.sampler = true, .color_target = true}
		};

		static constexpr uint32_t hiz_mip_levels = 9;

		/* Textures */

		graphics::Auto_texture depth_texture{depth_format, "Gbuffer Depth Texture"};  // Depth Texture

		graphics::Cycle_texture depth_value_texture{
			depth_value_format,
			"Gbuffer Cycled Depth Texture",
			hiz_mip_levels
		};  // Depth Value Texture

		graphics::Auto_texture albedo_texture{albedo_format, "Gbuffer Albedo Texture"};  // Albedo Texture

		graphics::Auto_texture lighting_info_texture{
			lighting_info_format,
			"Gbuffer Lighting Info Texture"
		};  // Lighting Info Texture

		/* Functions */

		// Resize all textures
		std::expected<void, util::Error> cycle(SDL_GPUDevice* device, glm::u32vec2 size) noexcept;
	};
}