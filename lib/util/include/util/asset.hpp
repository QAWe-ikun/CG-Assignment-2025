///
/// @file asset.hpp
/// @brief Provides a monad-friendly asset retrieval function
///

#pragma once

#include "error.hpp"

#include <expected>
#include <map>
#include <span>
#include <string>

namespace util
{
	///
	/// @brief Monad-friendly asset retrieval function
	///
	/// @param data Asset data
	/// @param name Name of the asset to retrieve
	/// @return The asset data as byte span, or an error if not found
	///
	std::expected<std::span<const std::byte>, util::Error> get_asset(
		const std::map<std::string, std::span<const std::byte>>& data,
		const std::string& name
	) noexcept;
}