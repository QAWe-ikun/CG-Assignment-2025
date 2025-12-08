///
/// @file find.hpp
/// @brief Proides monad-friendly helper functions to find elements in maps
///

#pragma once

#include <functional>
#include <map>
#include <optional>

namespace util
{
	///
	/// @brief A monad-friendly map find function
	///
	/// @tparam K Key type
	/// @tparam V Value type
	/// @tparam Q Query type
	/// @tparam Pred Comparator type of the map
	/// @tparam Alloc Allocator type of the map
	/// @param m The map to search
	/// @param key The key to search for
	/// @return A reference wrapper to the found value, or nullopt if not found
	///
	template <
		typename K,
		typename V,
		typename Q,
		typename Pred = std::less<K>,
		typename Alloc = std::allocator<std::pair<K, V>>
	>
	std::optional<std::reference_wrapper<const V>> find_map(
		const std::map<K, V, Pred, Alloc>& m,
		const Q& key
	) noexcept
	{
		const auto it = m.find(K(key));
		if (it == m.end()) return std::nullopt;
		return std::cref(it->second);
	}

	///
	/// @brief A monad-friendly map find function, `const` variant
	///
	/// @tparam K Key type
	/// @tparam V Value type
	/// @tparam Q Query type
	/// @tparam Pred Comparator type of the map
	/// @tparam Alloc Allocator type of the map
	/// @param m The map to search
	/// @param key The key to search for
	/// @return A reference wrapper to the found value, or nullopt if not found
	///
	template <
		typename K,
		typename V,
		typename Q,
		typename Pred = std::less<K>,
		typename Alloc = std::allocator<std::pair<K, V>>
	>
	std::optional<std::reference_wrapper<V>> find_map(std::map<K, V, Pred, Alloc>& m, const Q& key) noexcept
	{
		const auto it = m.find(K(key));
		if (it == m.end()) return std::nullopt;
		return std::ref(it->second);
	}
}