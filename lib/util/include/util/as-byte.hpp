#pragma once

#include <ranges>
#include <span>

namespace util
{
	template <std::ranges::contiguous_range T>
	inline std::span<const std::byte> as_bytes(const T& value) noexcept
	{
		return std::as_bytes(std::span(value));
	}

	template <typename T>
		requires(!std::ranges::contiguous_range<T>)
	inline std::span<const std::byte> as_bytes(const T& value) noexcept
	{
		return std::as_bytes(std::span(&value, 1));
	}

	template <std::ranges::contiguous_range T>
	inline std::span<std::byte> as_writable_bytes(T& value) noexcept
	{
		return std::as_writable_bytes(std::span(value));
	}
}