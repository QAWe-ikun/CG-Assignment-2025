#pragma once

#include "image/repr.hpp"
#include "util/error.hpp"

#include <expected>
#include <tiny_gltf.h>

namespace gltf::detail::image
{
	// Extract image as 8-bit RGBA
	std::expected<::image::Image<::image::Precision::U8, ::image::Format::RGBA>, util::Error> extract_u8_rgba(
		const tinygltf::Image& image
	) noexcept;

	// Extract image as 16-bit RGBA
	std::expected<::image::Image<::image::Precision::U16, ::image::Format::RGBA>, util::Error>
	extract_u16_rgba(const tinygltf::Image& image) noexcept;
}