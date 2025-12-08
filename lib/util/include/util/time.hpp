///
/// @file time.hpp
/// @brief Provides time measurement utilities
///

#pragma once

#include <chrono>
#include <functional>
#include <utility>

namespace util
{
	///
	/// @brief Measures the execution time of a callable object
	///
	/// @return Elapsed time in seconds and the result of the callable
	///
	template <typename Func, typename... Args>
		requires std::is_invocable_v<Func, Args&&...>
	std::pair<double, std::invoke_result_t<Func, Args&&...>> measure_time(Func&& func, Args&&... args)
	{
		const auto start = std::chrono::high_resolution_clock::now();
		auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
		const auto end = std::chrono::high_resolution_clock::now();

		const std::chrono::duration<double, std::ratio<1, 1>> duration = end - start;
		return {duration.count(), std::move(result)};
	}
}