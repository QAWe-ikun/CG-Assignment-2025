#include "image-io.hpp"

namespace image_io::internal
{
	template <>
	Load_result<uint8_t> load_from_memory<Precision::U8>(
		std::span<const std::byte> data,
		int desired_channel
	) noexcept
	{
		Load_result<uint8_t> result;
		result.pixels = stbi_load_from_memory(
			reinterpret_cast<const stbi_uc*>(data.data()),
			static_cast<int>(data.size()),
			&result.width,
			&result.height,
			&result.channels,
			desired_channel
		);
		return result;
	}

	template <>
	Load_result<uint16_t> load_from_memory<Precision::U16>(
		std::span<const std::byte> data,
		int desired_channel
	) noexcept
	{
		Load_result<uint16_t> result;
		result.pixels = stbi_load_16_from_memory(
			reinterpret_cast<const stbi_uc*>(data.data()),
			static_cast<int>(data.size()),
			&result.width,
			&result.height,
			&result.channels,
			desired_channel
		);
		return result;
	}

	template <>
	Load_result<float> load_from_memory<Precision::F32>(
		std::span<const std::byte> data,
		int desired_channel
	) noexcept
	{
		Load_result<float> result;
		result.pixels = stbi_loadf_from_memory(
			reinterpret_cast<const stbi_uc*>(data.data()),
			static_cast<int>(data.size()),
			&result.width,
			&result.height,
			&result.channels,
			desired_channel
		);
		return result;
	}
}