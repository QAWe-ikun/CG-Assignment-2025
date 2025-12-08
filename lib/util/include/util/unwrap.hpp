///
/// @file unwrap.hpp
/// @brief Useful unwrapping functions for std::expected with util::Error
///

#pragma once

#include <utility>

#include "error.hpp"
#include "inline.hpp"

namespace util
{
	struct Unwrap
	{
		std::source_location location;
		std::string message;
	};

	///
	/// @brief Error unwrapper
	/// @details Usage: `value = expected_value_or_error | util::unwrap("Error message")`
	///
	/// @param message Message for unwrapping error
	/// @param location Source location, defaults to current location
	/// @return Unwrapper object
	///
	FORCE_INLINE constexpr Unwrap unwrap(
		std::string message,
		const std::source_location& location = std::source_location::current()
	) noexcept
	{
		return Unwrap{.location = location, .message = std::move(message)};
	}

	FORCE_INLINE constexpr Unwrap unwrap(
		const std::source_location& location = std::source_location::current()
	) noexcept
	{
		return Unwrap{.location = location, .message = ""};
	}

	template <typename T>
		requires(!std::is_same_v<T, void>)
	FORCE_INLINE inline T operator|(std::expected<T, Error> expected, const Unwrap& unwrap)
	{
		if (!expected) [[unlikely]]
			throw expected.error().forward(unwrap.message, unwrap.location);
		return std::move(expected.value());
	}

	FORCE_INLINE inline void operator|(std::expected<void, Error> expected, const Unwrap& unwrap)
	{
		if (!expected) [[unlikely]]
			throw expected.error().forward(unwrap.message, unwrap.location);
	}
}