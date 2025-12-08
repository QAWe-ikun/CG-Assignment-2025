#include "gltf/detail/image/extract.hpp"

namespace gltf::detail::image
{
	template <typename T>
	static std::span<const T> as_span(const tinygltf::Image& image) noexcept
	{
		return std::span<const T>(reinterpret_cast<const T*>(image.image.data()), image.width * image.height);
	}

	std::expected<::image::Image<::image::Precision::U8, ::image::Format::RGBA>, util::Error> extract_u8_rgba(
		const tinygltf::Image& image
	) noexcept
	{
		const bool is_8bit = image.bits == 8 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
		const bool is_16bit = image.bits == 16 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;

		::image::Image<::image::Precision::U8, ::image::Format::RGBA> img{
			.size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height)),
			.pixels = std::vector<::image::Pixel_t<::image::Precision::U8, ::image::Format::RGBA>>(
				image.width * image.height
			)
		};

		if (image.component == 4)
		{
			if (is_8bit)
				std::ranges::copy(as_span<glm::u8vec4>(image), img.pixels.begin());
			else if (is_16bit)
				std::ranges::copy(
					as_span<glm::u16vec4>(image) | std::views::transform([](const glm::u16vec4& rgba) {
						return glm::u8vec4(rgba / uint16_t(256));
					}),
					img.pixels.begin()
				);
			else
				return util::Error(
					std::format(
						"Mismatched image bit depth ({}) or pixel type ({})",
						image.bits,
						image.pixel_type
					)
				);
		}
		else if (image.component == 3)
		{
			if (is_8bit)
				std::ranges::copy(
					as_span<glm::u8vec3>(image) | std::views::transform([](const glm::u8vec3& rgb) {
						return glm::u8vec4(rgb, 255);
					}),
					img.pixels.begin()
				);
			else if (is_16bit)
				std::ranges::copy(
					as_span<glm::u16vec3>(image) | std::views::transform([](const glm::u16vec3& rgb) {
						return glm::u8vec4(rgb / uint16_t(256), 255);
					}),
					img.pixels.begin()
				);
			else
				return util::Error(
					std::format(
						"Mismatched image bit depth ({}) or pixel type ({})",
						image.bits,
						image.pixel_type
					)
				);
		}
		else
			return util::Error(
				std::format("Unsupported number of components ({}) for color texture.", image.component)
			);

		return img;
	}

	std::expected<::image::Image<::image::Precision::U16, ::image::Format::RGBA>, util::Error>
	extract_u16_rgba(const tinygltf::Image& image) noexcept
	{
		if (image.bits != 16 || image.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			return util::Error(
				std::format(
					"Mismatched image bit depth ({}) or pixel type ({})",
					image.bits,
					image.pixel_type
				)
			);

		::image::Image<::image::Precision::U16, ::image::Format::RGBA> img{
			.size = glm::u32vec2(uint32_t(image.width), uint32_t(image.height)),
			.pixels = std::vector<::image::Pixel_t<::image::Precision::U16, ::image::Format::RGBA>>(
				image.width * image.height
			)
		};

		if (image.component == 4)
			std::ranges::copy(as_span<glm::u16vec4>(image), img.pixels.begin());
		else if (image.component == 3)
			std::ranges::copy(
				as_span<glm::u16vec3>(image) | std::views::transform([](const glm::u16vec3& rgb) {
					return glm::u8vec4(rgb, 255);
				}),
				img.pixels.begin()
			);
		else
			return util::Error(
				std::format("Unsupported number of components ({}) for color texture.", image.component)
			);

		return img;
	}
}