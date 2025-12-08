#pragma once

#include <glm/glm.hpp>

namespace gltf::detail::image
{
	// Check if dimension is power of two
	bool dim_power_of_2(uint32_t value) noexcept;

	// Check if image dimensions are power of two
	bool image_power_of_2(glm::u32vec2 size) noexcept;

	// Check if image size is multiple of block size (4x4)
	bool image_size_multiple_of_block(glm::u32vec2 size) noexcept;
}