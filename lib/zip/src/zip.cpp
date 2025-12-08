#include <format>
#include <gzip/decompress.hpp>
#include <stdexcept>

#include "zip/zip.hpp"

namespace zip
{
	std::expected<std::vector<std::byte>, util::Error> decompress(
		std::span<const std::byte> data,
		size_t max_size
	) noexcept
	{

		try
		{
			std::vector<std::byte> decompressed_data;
			decompressed_data.reserve(1024 * 1024);  // 预分配 1MB 空间以减少重新分配次数

			const gzip::Decompressor decompressor(max_size);

			decompressor.decompress(
				decompressed_data,
				reinterpret_cast<const char*>(data.data()),
				data.size()
			);

			return decompressed_data;
		}
		catch (const std::runtime_error& e)
		{
			return util::Error(std::format("解压失败: {}", e.what()));
		}
	}

	Decompressor::Decompressor(size_t max_size) :
		max_size(max_size)
	{}

	std::expected<std::vector<std::byte>, util::Error> Decompressor::operator()(
		std::span<const std::byte> data
	) const noexcept
	{
		return decompress(data, max_size);
	}

}