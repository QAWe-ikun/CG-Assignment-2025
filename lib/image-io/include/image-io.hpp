#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <glm/glm.hpp>
#include <span>
#include <stb_image.h>
#include <vector>

#include "util/error.hpp"

namespace image_io
{
	template <typename T>
	struct Image
	{
		using Pixel_type = T;

		glm::u32vec2 size;
		std::vector<T> pixels;
	};

	enum class Channel
	{
		Luminance_only = 1,
		RGB = 3,
		RGBA = 4
	};

	enum class Precision
	{
		U8,
		U16,
		F32
	};

	namespace internal
	{
		template <Precision P>
		struct Precision_type_mapping
		{
			using type = void;
		};

		template <>
		struct Precision_type_mapping<Precision::U8>
		{
			using type = std::uint8_t;
		};

		template <>
		struct Precision_type_mapping<Precision::U16>
		{
			using type = std::uint16_t;
		};

		template <>
		struct Precision_type_mapping<Precision::F32>
		{
			using type = float;
		};

		template <typename T>
		struct Load_result
		{
			T* pixels;
			int width;
			int height;
			int channels;
		};

		template <Precision P>
		Load_result<typename Precision_type_mapping<P>::type> load_from_memory(
			std::span<const std::byte> data [[maybe_unused]],
			int desired_channel [[maybe_unused]]
		) noexcept
		{
			static_assert(false, "Not implemented");
		}

		template <>
		Load_result<uint8_t> load_from_memory<Precision::U8>(
			std::span<const std::byte> data,
			int desired_channel
		) noexcept;

		template <>
		Load_result<uint16_t> load_from_memory<Precision::U16>(
			std::span<const std::byte> data,
			int desired_channel
		) noexcept;

		template <>
		Load_result<float> load_from_memory<Precision::F32>(
			std::span<const std::byte> data,
			int desired_channel
		) noexcept;

	}

	template <Channel C, Precision P>
	using Image_type =
		Image<glm::vec<static_cast<glm::length_t>(C), typename internal::Precision_type_mapping<P>::type>>;

	///
	/// @brief Load an image from memory
	///
	/// @tparam C Channels
	/// @tparam P Precision
	/// @param data Image data in memory
	/// @return Pixel data, or error information on failure
	///
	template <Channel C, Precision P>
	std::expected<Image_type<C, P>, util::Error> load_from_memory(std::span<const std::byte> data) noexcept
	{
		stbi_set_flip_vertically_on_load(1);
		const auto [pixels, width, height, channels] =
			internal::load_from_memory<P>(data, static_cast<int>(C));

		if (pixels == nullptr)
			return util::Error(std::format("Load image failed: {}", stbi_failure_reason()));

		Image_type<C, P> image{
			.size = {width, height},
			.pixels = std::vector(
				std::from_range,
				std::span(
					reinterpret_cast<const typename Image_type<C, P>::Pixel_type*>(pixels),
					size_t(width) * size_t(height)
				)
			)
		};

		stbi_image_free(pixels);

		return std::move(image);
	}
}