///
/// @file as-byte.hpp
/// @brief Provides helper functions to view objects/containers as byte spans
///

#pragma once

#include <ranges>
#include <span>

namespace util
{
	///
	/// @brief View the content of the given container as byte span
	///
	/// @param value The container to view
	/// @return A read-only byte span of the container
	///
	template <std::ranges::contiguous_range T>
	inline std::span<const std::byte> as_bytes(const T& value) noexcept
	{
		return std::as_bytes(std::span(value));
	}

	///
	/// @brief View the given object as byte span
	///
	/// @tparam T Type of the object
	///
	template <typename T>
		requires(!std::ranges::contiguous_range<T>)
	inline std::span<const std::byte> as_bytes(const T& value) noexcept
	{
		return std::as_bytes(std::span(&value, 1));
	}

	///
	/// @brief View the content of the given container as writable byte span
	///
	/// @tparam T Type of the container
	/// @param value The container to view
	/// @return A writable byte span of the container
	///
	template <std::ranges::contiguous_range T>
	inline std::span<std::byte> as_writable_bytes(T& value) noexcept
	{
		return std::as_writable_bytes(std::span(value));
	}
}