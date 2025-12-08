#pragma once

#include "error.hpp"

#include <expected>
#include <filesystem>
#include <vector>

namespace util
{
	///
	/// @brief Read a file into a byte vector
	///
	/// @param path File path
	/// @param max_size Maximum allowed size in bytes, files exceeding this size will return an error
	/// @return Byte vector containing file data, or error if failed
	///
	std::expected<std::vector<std::byte>, util::Error> read_file(
		const std::filesystem::path& path,
		size_t max_size = 256 * 1048576
	) noexcept;

	///
	/// @brief Write a file from a byte span
	///
	/// @param path File path
	/// @param data Data byte span
	///
	std::expected<void, util::Error> write_file(
		const std::filesystem::path& path,
		std::span<const std::byte> data
	) noexcept;
}